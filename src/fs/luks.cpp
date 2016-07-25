/*************************************************************************
 *  Copyright (C) 2012 by Volker Lanz <vl@fidra.de>                      *
 *  Copyright (C) 2013 by Andrius Štikonas <andrius@stikonas.eu>         *
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

#include "fs/filesystemfactory.h"

#include "util/capacity.h"
#include "util/externalcommand.h"
#include "util/report.h"

#include <QDebug>
#include <QDialog>
#include <QPointer>
#include <QRegularExpression>
#include <QString>
#include <QUuid>

#include <KDiskFreeSpaceInfo>
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
           const QString& label)
    : FileSystem(firstsector, lastsector, sectorsused, label, FileSystem::Luks)
    , m_innerFs(nullptr)
    , m_isCryptOpen(false)
    , m_cryptsetupFound(false)
    , m_isMounted(false)
    , m_logicalSectorSize(1)
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

bool luks::supportToolFound() const
{
    m_cryptsetupFound = findExternal(QStringLiteral("cryptsetup")) ? cmdSupportFileSystem : cmdSupportNone;
    return m_cryptsetupFound &&
        ((m_isCryptOpen && m_innerFs) ? m_innerFs->supportToolFound() : true);
}

FileSystem::SupportTool luks::supportToolName() const
{
    if (m_isCryptOpen && m_innerFs && m_cryptsetupFound)
        return m_innerFs->supportToolName();
    return SupportTool(QStringLiteral("cryptsetup"),
                       QUrl(QStringLiteral("https://code.google.com/p/cryptsetup/")));
}

bool luks::create(Report& report, const QString& deviceNode) const
{
    Q_ASSERT(m_innerFs);
    Q_ASSERT(!m_passphrase.isEmpty());

    ExternalCommand createCmd(report, QStringLiteral("cryptsetup"),
                              { QStringLiteral("-s"),
                                QStringLiteral("512"),
                                QStringLiteral("--batch-mode"),
                                QStringLiteral("luksFormat"),
                                deviceNode });
    if (!( createCmd.start(-1) &&
                createCmd.write(m_passphrase.toUtf8() + '\n') == m_passphrase.toUtf8().length() + 1 &&
                createCmd.waitFor() && createCmd.exitCode() == 0))
    {
        return false;
    }

    ExternalCommand openCmd(report, QStringLiteral("cryptsetup"),
                              { QStringLiteral("open"),
                                deviceNode,
                                suggestedMapperName(deviceNode) });

    if (!( openCmd.start(-1) &&  openCmd.write(m_passphrase.toUtf8() + '\n') == m_passphrase.toUtf8().length() + 1 && openCmd.waitFor()))
        return false;

    QString mapperNode = mapperName(deviceNode);
    if (mapperNode.isEmpty())
        return false;

    if (!m_innerFs->create(report, mapperNode))
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

bool luks::canMount(const QString& deviceNode, const QString& mountPoint) const
{
    return m_isCryptOpen &&
           !m_isMounted &&
           m_innerFs &&
           m_innerFs->canMount(mapperName(deviceNode), mountPoint);
}

bool luks::canUnmount(const QString& deviceNode) const
{
    return m_isCryptOpen &&
           m_isMounted &&
           m_innerFs &&
           m_innerFs->canUnmount(mapperName(deviceNode));
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
        if (!mapperName(deviceNode).isEmpty())
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
                                deviceNode,
                                suggestedMapperName(deviceNode) });

    if (!( openCmd.start(-1) &&
                    openCmd.write(passphrase.toUtf8() + '\n') == passphrase.toUtf8().length() + 1 &&
                    openCmd.waitFor() && openCmd.exitCode() == 0) )
        return false;

    if (m_innerFs)
    {
        delete m_innerFs;
        m_innerFs = nullptr;
    }

    QString mapperNode = mapperName(deviceNode);
    if (mapperNode.isEmpty())
        return false;

    loadInnerFileSystem(deviceNode, mapperNode);
    m_isCryptOpen = (m_innerFs != nullptr);

    if (!m_isCryptOpen)
        return false;

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
                        { QStringLiteral("close"), mapperName(deviceNode) });
    if (!(cmd.run(-1) && cmd.exitCode() == 0))
        return false;

    delete m_innerFs;
    m_innerFs = nullptr;

    m_passphrase.clear();
    setLabel({});
    setUUID(readUUID(deviceNode));
    setSectorsUsed(-1);

    m_isCryptOpen = (m_innerFs != nullptr);

    if (m_isCryptOpen)
        return false;
    return true;
}

void luks::loadInnerFileSystem(const QString& deviceNode, const QString& mapperNode)
{
    Q_ASSERT(!m_innerFs);
    FileSystem::Type innerFsType = detectFileSystem(mapperNode);
    m_innerFs = FileSystemFactory::cloneWithNewType(innerFsType,
                                                    *this);
    setLabel(m_innerFs->readLabel(mapperNode));
    setUUID(m_innerFs->readUUID(mapperNode));
    if (m_innerFs->supportGetUsed() == FileSystem::cmdSupportFileSystem)
        setSectorsUsed((m_innerFs->readUsedCapacity(mapperNode) + getPayloadOffset(deviceNode)) / m_logicalSectorSize );
}

void luks::createInnerFileSystem(FileSystem::Type type)
{
    Q_ASSERT(!m_innerFs);
    m_innerFs = FileSystemFactory::cloneWithNewType(type, *this);
}

bool luks::check(Report& report, const QString& deviceNode) const
{
    Q_ASSERT(m_innerFs);

    QString mapperNode = mapperName(deviceNode);
    if (mapperNode.isEmpty())
        return false;

    return m_innerFs->check(report, mapperNode);
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

    QString mapperNode = mapperName(deviceNode);
    if (mapperNode.isEmpty())
        return false;

    if (m_innerFs->canMount(mapperNode, mountPoint))
    {
        if (m_innerFs->mount(report, mapperNode, mountPoint))
        {
            m_isMounted = true;

            const KDiskFreeSpaceInfo freeSpaceInfo = KDiskFreeSpaceInfo::freeSpaceInfo(mountPoint);
            if (freeSpaceInfo.isValid() && mountPoint != QString())
                setSectorsUsed((freeSpaceInfo.used() + getPayloadOffset(deviceNode)) / m_logicalSectorSize);

            return true;
        }
    }
    else {
        ExternalCommand mountCmd(
                report,
                QStringLiteral("mount"),
                { QStringLiteral("--verbose"), mapperNode, mountPoint });
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

    QString mapperNode = mapperName(deviceNode);
    if (mapperNode.isEmpty())
        return false;

    if (m_innerFs->canUnmount(mapperNode))
    {
        if (m_innerFs->unmount(report, mapperNode))
        {
            m_isMounted = false;
            return true;
        }
    }
    else {
        ExternalCommand unmountCmd( report,
                QStringLiteral("umount"),
                { QStringLiteral("--verbose"), QStringLiteral("--all-targets"), mapperNode });
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
        return m_innerFs->readLabel(mapperName(deviceNode));
    return QString();
}

bool luks::writeLabel(Report& report, const QString& deviceNode, const QString& newLabel)
{
    Q_ASSERT(m_innerFs);
    return m_innerFs->writeLabel(report, mapperName(deviceNode), newLabel);
}

bool luks::resize(Report& report, const QString& deviceNode, qint64 newLength) const
{
    Q_ASSERT(m_innerFs);

    QString mapperNode = mapperName(deviceNode);
    if (mapperNode.isEmpty())
        return false;

    qint64 payloadLength = newLength - getPayloadOffset(deviceNode);
    if ( newLength - length() * m_logicalSectorSize > 0 )
    {
        ExternalCommand cryptResizeCmd(report, QStringLiteral("cryptsetup"), { QStringLiteral("resize"), mapperNode });
        report.line() << xi18nc("@info:progress", "Resizing LUKS crypt on partition <filename>%1</filename>.", deviceNode);

        if (cryptResizeCmd.run(-1) && cryptResizeCmd.exitCode() == 0)
        {
            return m_innerFs->resize(report, mapperNode, payloadLength);
        }
    }
    else if (m_innerFs->resize(report, mapperNode, payloadLength))
    {
        ExternalCommand cryptResizeCmd(report, QStringLiteral("cryptsetup"), 
                {  QStringLiteral("--size"), QString::number(payloadLength / /*m_logicalSectorSize*/ 512), // LUKS assume 512 bytes sector
                   QStringLiteral("resize"), mapperNode });
        report.line() << xi18nc("@info:progress", "Resizing LUKS crypt on partition <filename>%1</filename>.", deviceNode);
        if (cryptResizeCmd.run(-1) && cryptResizeCmd.exitCode() == 0)
        {
            return true;
        }
    }
    report.line() << xi18nc("@info:progress", "Resizing encrypted file system on partition <filename>%1</filename> failed.", deviceNode);
    return false;
}

QString luks::readUUID(const QString& deviceNode) const
{
    if (m_isCryptOpen && m_innerFs)
        return m_innerFs->readUUID(mapperName(deviceNode));
    return readOuterUUID(deviceNode);
}

QString luks::readOuterUUID(const QString &deviceNode) const
{
    ExternalCommand cmd(QStringLiteral("cryptsetup"),
                        { QStringLiteral("luksUUID"), deviceNode });
    if (cmd.run()) {
        return cmd.output().trimmed();
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

QString luks::mapperName(const QString& deviceNode)
{
    ExternalCommand cmd(QStringLiteral("lsblk"),
                        { QStringLiteral("--list"),
                          QStringLiteral("--noheadings"),
                          QStringLiteral("--output"),
                          QStringLiteral("name"),
                          deviceNode });
    if (cmd.run(-1) && cmd.exitCode() == 0) {
        QStringList output=cmd.output().split(QStringLiteral("\n"));
        output.removeFirst();
        if (!output.first().isEmpty())
            return QStringLiteral("/dev/mapper/") + output.first();
    }
    return QString();
}

QString luks::getCipherName(const QString& deviceNode) const
{
    ExternalCommand cmd(QStringLiteral("cryptsetup"),
                        { QStringLiteral("luksDump"), deviceNode });
    if (cmd.run(-1) && cmd.exitCode() == 0) {
        QRegularExpression re(QStringLiteral("Cipher name:\\s+(\\w+)"));
        QRegularExpressionMatch reCipherName = re.match(cmd.output());
        if (reCipherName.hasMatch())
            return reCipherName.captured(1);
    }
    return QStringLiteral("---");
}

QString luks::getCipherMode(const QString& deviceNode) const
{
    ExternalCommand cmd(QStringLiteral("cryptsetup"),
                        { QStringLiteral("luksDump"), deviceNode });
    if (cmd.run(-1) && cmd.exitCode() == 0) {
        QRegularExpression re(QStringLiteral("Cipher mode:\\s+(\\w+)"));
        QRegularExpressionMatch reCipherMode = re.match(cmd.output());
        if (reCipherMode.hasMatch())
            return reCipherMode.captured(1);
    }
    return QStringLiteral("---");
}

QString luks::getHashName(const QString& deviceNode) const
{
    ExternalCommand cmd(QStringLiteral("cryptsetup"),
                        { QStringLiteral("luksDump"), deviceNode });
    if (cmd.run(-1) && cmd.exitCode() == 0) {
        QRegularExpression re(QStringLiteral("Hash spec:\\s+(\\w+)"));
        QRegularExpressionMatch reHash = re.match(cmd.output());
        if (reHash.hasMatch())
            return reHash.captured(1);
    }
    return QStringLiteral("---");
}

qint64 luks::getKeySize(const QString& deviceNode) const
{
    ExternalCommand cmd(QStringLiteral("cryptsetup"),
                        { QStringLiteral("luksDump"), deviceNode });
    if (cmd.run(-1) && cmd.exitCode() == 0) {
        QRegularExpression re(QStringLiteral("MK bits:\\s+(\\d+)"));
        QRegularExpressionMatch reKeySize = re.match(cmd.output());
        if (reKeySize.hasMatch())
            return reKeySize.captured(1).toLongLong();
    }
    return -1;
}

/*
 * @return size of payload offset in bytes.
 */
qint64 luks::getPayloadOffset(const QString& deviceNode) const
{
    //4096 sectors and 512 bytes.
    return 4096 * 512;
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

}
