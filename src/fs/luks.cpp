/*************************************************************************
 *  Copyright (C) 2012 by Volker Lanz <vl@fidra.de>                      *
 *  Copyright (C) 2013-2017 by Andrius Štikonas <andrius@stikonas.eu>    *
 *  Copyright (C) 2015-2016 by Teo Mrnjavac <teo@kde.org>                *
 *                                                                       *
 *  This program is free software; you can redistribute it and/or        *
 *  modify it under the terms of the GNU General Public License as       *
 *  published by the Free Software Foundation; either version 3 of       *
 *  the License, or (at your option) any later version.                  *
 *                                                                       *
 *  This program is distributed in the hope that it will be useful,      *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of       *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        *
 *  GNU General Public License for more details.                         *
 *                                                                       *
 *  You should have received a copy of the GNU General Public License    *
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.*
 *************************************************************************/

#include "fs/luks.h"
#include "fs/lvm2_pv.h"

#include "fs/filesystemfactory.h"

#include "util/externalcommand.h"
#include "util/capacity.h"
#include "util/helpers.h"
#include "util/report.h"

#include <cmath>

#include <QDebug>
#include <QDialog>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QRegularExpression>
#include <QPointer>
#include <QStorageInfo>
#include <QString>
#include <QUuid>

#include <KLocalizedString>
#include <KPasswordDialog>

namespace FS
{
FileSystem::CommandSupportType luks::m_GetUsed = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType luks::m_GetLabel = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType luks::m_Create = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType luks::m_Grow = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType luks::m_Shrink = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType luks::m_Move = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType luks::m_Check = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType luks::m_Copy = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType luks::m_Backup = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType luks::m_SetLabel = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType luks::m_UpdateUUID = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType luks::m_GetUUID = FileSystem::cmdSupportNone;

luks::luks(qint64 firstsector,
           qint64 lastsector,
           qint64 sectorsused,
           const QString& label,
           FileSystem::Type t)
    : FileSystem(firstsector, lastsector, sectorsused, label, t)
    , m_innerFs(nullptr)
    , m_isCryptOpen(false)
    , m_cryptsetupFound(m_Create != cmdSupportNone)
    , m_isMounted(false)
    , m_KeySize(-1)
    , m_PayloadOffset(-1)
{
}

luks::~luks()
{
    delete m_innerFs;
}

void luks::init()
{
    CommandSupportType cryptsetupFound = findExternal(QStringLiteral("cryptsetup")) ? cmdSupportFileSystem : cmdSupportNone;

    m_Create     = cryptsetupFound;
    m_UpdateUUID = cryptsetupFound;
    m_GetUUID    = cryptsetupFound;
    m_Grow       = cryptsetupFound;
    m_Shrink     = cryptsetupFound;

    m_SetLabel = cmdSupportNone;
    m_GetLabel = cmdSupportFileSystem;
    m_Check = cmdSupportCore;
    m_Copy = cmdSupportCore;
    m_Move = cmdSupportCore;
    m_Backup = cmdSupportCore;
    m_GetUsed = cmdSupportNone; // libparted does not support LUKS, we do this as a special case
}

void luks::scan(const QString& deviceNode)
{
    getMapperName(deviceNode);
    getLuksInfo(deviceNode);
}

bool luks::supportToolFound() const
{
    return m_cryptsetupFound && ((m_isCryptOpen && m_innerFs) ? m_innerFs->supportToolFound() : true);
}

FileSystem::SupportTool luks::supportToolName() const
{
    if (m_isCryptOpen && m_innerFs && m_cryptsetupFound)
        return m_innerFs->supportToolName();
    return SupportTool(QStringLiteral("cryptsetup"),
                       QUrl(QStringLiteral("https://code.google.com/p/cryptsetup/")));
}

bool luks::create(Report& report, const QString& deviceNode)
{
    Q_ASSERT(m_innerFs);
    Q_ASSERT(!m_passphrase.isEmpty());

    ExternalCommand createCmd(report, QStringLiteral("cryptsetup"),
                              { QStringLiteral("-s"),
                                QStringLiteral("512"),
                                QStringLiteral("--batch-mode"),
                                QStringLiteral("--force-password"),
                                QStringLiteral("--type"), QStringLiteral("luks1"),
                                QStringLiteral("luksFormat"),
                                deviceNode });
    if (!( createCmd.start(-1) &&
                createCmd.write(m_passphrase.toLocal8Bit() + '\n') == m_passphrase.toLocal8Bit().length() + 1 &&
                createCmd.waitFor() && createCmd.exitCode() == 0))
    {
        return false;
    }

    ExternalCommand openCmd(report, QStringLiteral("cryptsetup"),
                              { QStringLiteral("open"),
                                deviceNode,
                                suggestedMapperName(deviceNode) });

    if (!( openCmd.start(-1) &&  openCmd.write(m_passphrase.toLocal8Bit() + '\n') == m_passphrase.toLocal8Bit().length() + 1 && openCmd.waitFor()))
        return false;

    setPayloadSize();
    scan(deviceNode);

    if (mapperName().isEmpty())
        return false;

    if (!m_innerFs->create(report, mapperName()))
        return false;

    return true;
}

QString luks::mountTitle() const
{
    return xi18nc("@title:menu", "Mount");
}

QString luks::unmountTitle() const
{
    return xi18nc("@title:menu", "Unmount");
}

QString luks::cryptOpenTitle() const
{
    return xi18nc("@title:menu", "Decrypt");
}

QString luks::cryptCloseTitle() const
{
    return xi18nc("@title:menu", "Deactivate");
}

void luks::setPassphrase(const QString& passphrase)
{
    m_passphrase = passphrase;
}

QString luks::passphrase() const
{
    return m_passphrase;
}

bool luks::canMount(const QString&, const QString& mountPoint) const
{
    return m_isCryptOpen &&
           !m_isMounted &&
           m_innerFs &&
           m_innerFs->canMount(mapperName(), mountPoint);
}

bool luks::canUnmount(const QString&) const
{
    return m_isCryptOpen &&
           m_isMounted &&
           m_innerFs &&
           m_innerFs->canUnmount(mapperName());
}

bool luks::isMounted() const
{
    return m_isCryptOpen && m_isMounted;
}

void luks::setMounted(bool mounted)
{
    m_isMounted = mounted;
}

bool luks::canCryptOpen(const QString&) const
{
    return !m_isCryptOpen && !m_isMounted && supportToolFound();
}

bool luks::canCryptClose(const QString&) const
{
    return m_isCryptOpen && !m_isMounted && m_cryptsetupFound;
}

bool luks::isCryptOpen() const
{
    return m_isCryptOpen;
}

void luks::setCryptOpen(bool cryptOpen)
{
    m_isCryptOpen = cryptOpen;
}

bool luks::cryptOpen(QWidget* parent, const QString& deviceNode)
{
    if (m_isCryptOpen)
    {
        if (!mapperName().isEmpty())
        {
            qWarning() << "LUKS device" << deviceNode
                       << "already decrypted."
                       << "Cannot decrypt again.";
            return false;
        }
        else
        {
            qWarning() << "LUKS device" << deviceNode
                       << "reportedly decrypted but mapper node not found."
                       << "Marking device as NOT decrypted and trying to "
                          "decrypt again anyway.";
            m_isCryptOpen = false;
        }
    }

    KPasswordDialog dlg( parent );
    dlg.setPrompt(i18n("Enter passphrase for %1:", deviceNode));
    if( !dlg.exec() )
        return false;

    QString passphrase = dlg.password();
    ExternalCommand openCmd(QStringLiteral("cryptsetup"),
                              { QStringLiteral("open"),
                                QStringLiteral("--tries"), QStringLiteral("1"),
                                deviceNode,
                                suggestedMapperName(deviceNode) });

    if (!( openCmd.start(-1) &&
                    openCmd.write(passphrase.toLocal8Bit() + '\n') == passphrase.toLocal8Bit().length() + 1 &&
                    openCmd.waitFor() && openCmd.exitCode() == 0) )
        return false;

    if (m_innerFs) {
        delete m_innerFs;
        m_innerFs = nullptr;
    }

    scan(deviceNode);

    if (mapperName().isEmpty())
        return false;

    loadInnerFileSystem(mapperName());
    m_isCryptOpen = (m_innerFs != nullptr);

    if (!m_isCryptOpen)
        return false;

    for (auto &p : LVM::pvList)
        if (p.isLuks() && p.partition()->deviceNode() == deviceNode && p.partition()->fileSystem().type() == FileSystem::Lvm2_PV)
            p.setLuks(false);

    m_passphrase = passphrase;
    return true;
}

bool luks::cryptClose(const QString& deviceNode)
{
    if (!m_isCryptOpen)
    {
        qWarning() << "Cannot close LUKS device" << deviceNode
                   << "because it's not open.";
        return false;
    }

    if (m_isMounted)
    {
        qWarning() << "Cannot close LUKS device" << deviceNode
                   << "because the filesystem is mounted.";
        return false;
    }

    ExternalCommand cmd(QStringLiteral("cryptsetup"),
                        { QStringLiteral("close"), mapperName() });
    if (!(cmd.run(-1) && cmd.exitCode() == 0))
        return false;

    delete m_innerFs;
    m_innerFs = nullptr;

    m_passphrase.clear();
    setLabel(FileSystem::readLabel(deviceNode));
    setUUID(readUUID(deviceNode));
    setSectorsUsed(-1);

    m_isCryptOpen = (m_innerFs != nullptr);

    for (auto &p : LVM::pvList)
        if (!p.isLuks() && p.partition()->deviceNode() == deviceNode)
            p.setLuks(true);

    return true;
}

void luks::loadInnerFileSystem(const QString& mapperNode)
{
    Q_ASSERT(!m_innerFs);
    FileSystem::Type innerFsType = detectFileSystem(mapperNode);
    m_innerFs = FileSystemFactory::cloneWithNewType(innerFsType,
                                                    *this);
    setLabel(m_innerFs->readLabel(mapperNode));
    setUUID(m_innerFs->readUUID(mapperNode));
    if (m_innerFs->supportGetUsed() == FileSystem::cmdSupportFileSystem)
        setSectorsUsed(static_cast<qint64>(std::ceil((m_innerFs->readUsedCapacity(mapperNode) + payloadOffset()) / static_cast<double>(sectorSize()) )));
    m_innerFs->scan(mapperNode);
}

void luks::createInnerFileSystem(FileSystem::Type type)
{
    Q_ASSERT(!m_innerFs);
    m_innerFs = FileSystemFactory::cloneWithNewType(type, *this);
}

bool luks::check(Report& report, const QString&) const
{
    Q_ASSERT(m_innerFs);

    if (mapperName().isEmpty())
        return false;

    return m_innerFs->check(report, mapperName());
}

qint64 luks::readUsedCapacity(const QString& deviceNode) const
{
    if (!m_isCryptOpen)
        return -1;
    if (m_innerFs)
        return m_innerFs->readUsedCapacity(deviceNode);
    return -1;
}

bool luks::mount(Report& report, const QString& deviceNode, const QString& mountPoint)
{
    if (!m_isCryptOpen)
    {
        qWarning() << "Cannot mount device" << deviceNode
                   << "before decrypting it first.";
        return false;
    }

    if (m_isMounted)
    {
        qWarning() << "Cannot mount device" << deviceNode
                   << "because it's already mounted.";
        return false;
    }

    Q_ASSERT(m_innerFs);

    if (mapperName().isEmpty())
        return false;

    if (m_innerFs->canMount(mapperName(), mountPoint))
    {
        if (m_innerFs->mount(report, mapperName(), mountPoint))
        {
            m_isMounted = true;

            const QStorageInfo storageInfo = QStorageInfo(mountPoint);
            if (storageInfo.isValid() && !mountPoint.isEmpty())
                setSectorsUsed( (storageInfo.bytesTotal() - storageInfo.bytesFree() + payloadOffset()) / sectorSize());

            return true;
        }
    }
    else {
        ExternalCommand mountCmd(
                report,
                QStringLiteral("mount"),
                { QStringLiteral("--verbose"), mapperName(), mountPoint });
        if (mountCmd.run() && mountCmd.exitCode() == 0)
        {
            m_isMounted = true;
            return true;
        }
    }
    return false;
}

bool luks::unmount(Report& report, const QString& deviceNode)
{
    if (!m_isCryptOpen)
    {
        qWarning() << "Cannot unmount device" << deviceNode
                   << "before decrypting it first.";
        return false;
    }

    if (!m_isMounted)
    {
        qWarning() << "Cannot unmount device" << deviceNode
                   << "because it's not mounted.";
        return false;
    }

    Q_ASSERT(m_innerFs);

    if (mapperName().isEmpty())
        return false;

    if (m_innerFs->canUnmount(mapperName()))
    {
        if (m_innerFs->unmount(report, mapperName()))
        {
            m_isMounted = false;
            return true;
        }
    }
    else {
        ExternalCommand unmountCmd( report,
                QStringLiteral("umount"),
                { QStringLiteral("--verbose"), QStringLiteral("--all-targets"), mapperName() });
        if (unmountCmd.run() && unmountCmd.exitCode() == 0)
        {
            m_isMounted = false;
            return true;
        }
    }
    return false;
}

FileSystem::Type luks::type() const
{
    if (m_isCryptOpen && m_innerFs)
        return m_innerFs->type();
    return FileSystem::Luks;
}

QString luks::suggestedMapperName(const QString& deviceNode) const
{
    return QStringLiteral("luks-") + readOuterUUID(deviceNode);
}

QString luks::readLabel(const QString& deviceNode) const
{
    if (m_isCryptOpen && m_innerFs)
        return m_innerFs->readLabel(mapperName());

    return FileSystem::readLabel(deviceNode);
}

bool luks::writeLabel(Report& report, const QString&, const QString& newLabel)
{
    Q_ASSERT(m_innerFs);
    return m_innerFs->writeLabel(report, mapperName(), newLabel);
}

bool luks::resize(Report& report, const QString& deviceNode, qint64 newLength) const
{
    Q_ASSERT(m_innerFs);

    if (mapperName().isEmpty())
        return false;

    if ( newLength - length() * sectorSize() > 0 )
    {
        ExternalCommand cryptResizeCmd(report, QStringLiteral("cryptsetup"), { QStringLiteral("resize"), mapperName() });
        report.line() << xi18nc("@info:progress", "Resizing LUKS crypt on partition <filename>%1</filename>.", deviceNode);

        if (cryptResizeCmd.run(-1) && cryptResizeCmd.exitCode() == 0)
            return m_innerFs->resize(report, mapperName(), m_PayloadSize);
    }
    else if (m_innerFs->resize(report, mapperName(), m_PayloadSize))
    {
        ExternalCommand cryptResizeCmd(report, QStringLiteral("cryptsetup"),
                {  QStringLiteral("--size"), QString::number(m_PayloadSize / 512), // LUKS1 payload length is specified in multiples of 512 bytes
                   QStringLiteral("resize"), mapperName() });
        report.line() << xi18nc("@info:progress", "Resizing LUKS crypt on partition <filename>%1</filename>.", deviceNode);
        if (cryptResizeCmd.run(-1) && cryptResizeCmd.exitCode() == 0)
            return true;
    }
    report.line() << xi18nc("@info:progress", "Resizing encrypted file system on partition <filename>%1</filename> failed.", deviceNode);
    return false;
}

bool luks::resizeOnline(Report& report, const QString& deviceNode, const QString& mountPoint, qint64 length) const
{
    Q_UNUSED(mountPoint)
    return resize(report, deviceNode, length);
}

QString luks::readUUID(const QString& deviceNode) const
{
    QString outerUuid = readOuterUUID(deviceNode);
    if (m_isCryptOpen && m_innerFs)
        return m_innerFs->readUUID(mapperName());
    return outerUuid;
}

QString luks::readOuterUUID(const QString &deviceNode) const
{
    if ( deviceNode.isEmpty() )
        return QString();

    ExternalCommand cmd(QStringLiteral("cryptsetup"),
                        { QStringLiteral("luksUUID"), deviceNode });
    if (cmd.run()) {
        if ( cmd.exitCode() )
        {
            qWarning() << "Cannot get luksUUID for device" << deviceNode
                       << "\tcryptsetup exit code" << cmd.exitCode()
                       << "\toutput:" << cmd.output().trimmed();
            return QString();
        }
        QString outerUuid = cmd.output().trimmed();
        const_cast< QString& >( m_outerUuid ) = outerUuid;
        return outerUuid;
    }
    return QStringLiteral("---");
}

bool luks::updateUUID(Report& report, const QString& deviceNode) const
{
    const QString uuid = QUuid::createUuid().toString().remove(QRegularExpression(QStringLiteral("\\{|\\}")));

    ExternalCommand cmd(report,
                        QStringLiteral("cryptsetup"),
                        { QStringLiteral("luksUUID"),
                          deviceNode,
                          QStringLiteral("--uuid"),
                          uuid });
    return cmd.run(-1) && cmd.exitCode() == 0;
}

void luks::getMapperName(const QString& deviceNode)
{
    ExternalCommand cmd(QStringLiteral("lsblk"),
                        { QStringLiteral("--list"),
                          QStringLiteral("--noheadings"),
                          QStringLiteral("--paths"),
                          QStringLiteral("--json"),
                          QStringLiteral("--output"),
                          QStringLiteral("type,name"),
                          deviceNode });
    m_MapperName = QString();

    if (cmd.run(-1) && cmd.exitCode() == 0) {
        const QJsonDocument jsonDocument = QJsonDocument::fromJson(cmd.rawOutput());
        QJsonObject jsonObject = jsonDocument.object();
        const QJsonArray jsonArray = jsonObject[QLatin1String("blockdevices")].toArray();
        for (const auto &deviceLine : jsonArray) {
            QJsonObject deviceObject = deviceLine.toObject();
            if (deviceObject[QLatin1String("type")].toString() == QLatin1String("crypt")) {
                m_MapperName = deviceObject[QLatin1String("name")].toString();
                break;
            }
        }
    }
}

void luks::getLuksInfo(const QString& deviceNode)
{
    ExternalCommand cmd(QStringLiteral("cryptsetup"), { QStringLiteral("luksDump"), deviceNode });
    if (cmd.run(-1) && cmd.exitCode() == 0) {
        QRegularExpression re(QStringLiteral("Cipher name:\\s+(\\w+)"));
        QRegularExpressionMatch rem = re.match(cmd.output());
        if (rem.hasMatch())
            m_CipherName = rem.captured(1);
        else
            m_CipherName = QLatin1String("---");

        re.setPattern(QStringLiteral("Cipher mode:\\s+(\\w+)"));
        rem = re.match(cmd.output());
        if (rem.hasMatch())
            m_CipherMode = rem.captured(1);
        else
            m_CipherMode = QLatin1String("---");

        re.setPattern(QStringLiteral("Hash spec:\\s+(\\w+)"));
        rem = re.match(cmd.output());
        if (rem.hasMatch())
            m_HashName = rem.captured(1);
        else
            m_HashName = QLatin1String("---");

        re.setPattern(QStringLiteral("MK bits:\\s+(\\d+)"));
        rem = re.match(cmd.output());
        if (rem.hasMatch())
            m_KeySize = rem.captured(1).toLongLong();
        else
            m_KeySize = -1;

        re.setPattern(QStringLiteral("Payload offset:\\s+(\\d+)"));
        rem = re.match(cmd.output());
        if (rem.hasMatch())
            m_PayloadOffset = rem.captured(1).toLongLong() * 512; // assuming LUKS sector size is 512;
        else
            m_PayloadOffset = -1;

    }
    else {
        m_CipherName = QLatin1String("---");
        m_CipherMode = QLatin1String("---");
        m_HashName = QLatin1String("---");
        m_KeySize = -1;
        m_PayloadOffset = -1;
    }
}

QString luks::outerUuid() const
{
    return m_outerUuid;
}

bool luks::canEncryptType(FileSystem::Type type)
{
    switch (type)
    {
        case Btrfs:
        case F2fs:
        case Ext2:
        case Ext3:
        case Ext4:
        case Jfs:
        case LinuxSwap:
        case Lvm2_PV:
        case Nilfs2:
        case ReiserFS:
        case Reiser4:
        case Xfs:
        case Zfs:
            return true;
        default:
            return false;
    }
}

void luks::initLUKS()
{
    setPayloadSize();
    QString mapperNode = mapperName();
    bool isCryptOpen = !mapperNode.isEmpty();
    setCryptOpen(isCryptOpen);
    if (isCryptOpen) {
        loadInnerFileSystem(mapperNode);
        setMounted(detectMountStatus(innerFS(), mapperNode));
    }
}

void luks::setPayloadSize()
{
    ExternalCommand dmsetupCmd(QStringLiteral("dmsetup"), { QStringLiteral("table"), mapperName() });
    dmsetupCmd.run();
    QRegularExpression re(QStringLiteral("\\d+ (\\d+)"));
    QRegularExpressionMatch rem = re.match(dmsetupCmd.output());
    if (rem.hasMatch())
        m_PayloadSize = rem.captured(1).toLongLong() * sectorSize();
}

bool luks::testPassphrase(const QString& deviceNode, const QString& passphrase) const {
    ExternalCommand cmd(QStringLiteral("cryptsetup"), { QStringLiteral("open"), QStringLiteral("--tries"), QStringLiteral("1"), QStringLiteral("--test-passphrase"), deviceNode });
    if (cmd.start(-1) && cmd.write(passphrase.toLocal8Bit() + '\n') == passphrase.toLocal8Bit().length() + 1 && cmd.waitFor() && cmd.exitCode() == 0)
        return true;

    return false;
}

}
