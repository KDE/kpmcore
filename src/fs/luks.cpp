/*************************************************************************
 *  Copyright (C) 2012 by Volker Lanz <vl@fidra.de>                      *
 *  Copyright (C) 2013 by Andrius Štikonas <andrius@stikonas.eu>         *
 *  Copyright (C) 2015 by Teo Mrnjavac <teo@kde.org>                     *
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

#include "gui/decryptluksdialog.h"

#include "util/capacity.h"
#include "util/externalcommand.h"

#include <QDebug>
#include <QDialog>
#include <QPointer>
#include <QString>
#include <QUuid>

#include <KLocalizedString>

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
    , m_isMounted(false)
{
}

luks::~luks()
{
    delete m_innerFs;
}

void luks::init()
{
    m_UpdateUUID = findExternal(QStringLiteral("cryptsetup")) ? cmdSupportFileSystem : cmdSupportNone;
    m_Copy = cmdSupportCore;
    m_Move = cmdSupportCore;
    m_Backup = cmdSupportCore;
    m_GetUUID = findExternal(QStringLiteral("cryptsetup")) ? cmdSupportFileSystem : cmdSupportNone;
}

bool luks::supportToolFound() const
{
    return
//          m_GetUsed != cmdSupportNone &&
//          m_GetLabel != cmdSupportNone &&
//          m_SetLabel != cmdSupportNone &&
//          m_Create != cmdSupportNone &&
//          m_Check != cmdSupportNone &&
        m_UpdateUUID != cmdSupportNone &&
//          m_Grow != cmdSupportNone &&
//          m_Shrink != cmdSupportNone &&
        m_Copy != cmdSupportNone &&
        m_Move != cmdSupportNone &&
        m_Backup != cmdSupportNone &&
        m_GetUUID != cmdSupportNone;
}

FileSystem::SupportTool luks::supportToolName() const
{
    return SupportTool(QStringLiteral("cryptsetup"), QUrl(QStringLiteral("https://code.google.com/p/cryptsetup/")));
}

qint64 luks::minCapacity() const
{
    return 3 * Capacity::unitFactor(Capacity::Byte, Capacity::MiB);
}

QString luks::mountTitle() const
{
    return i18nc("@title:menu", "Mount");
}

QString luks::unmountTitle() const
{
    return i18nc("@title:menu", "Unmount");
}

QString luks::cryptOpenTitle() const
{
    return i18nc("@title:menu", "Decrypt");
}

QString luks::cryptCloseTitle() const
{
    return i18nc("@title:menu", "Deactivate");
}

bool luks::canMount(const QString& deviceNode) const
{
    return m_isCryptOpen &&
           !m_isMounted &&
           m_innerFs &&
           m_innerFs->canMount(deviceNode);
}

bool luks::canUnmount(const QString& deviceNode) const
{
    return m_isCryptOpen &&
           m_isMounted &&
           m_innerFs &&
           m_innerFs->canUnmount(deviceNode);
}

bool luks::isMounted() const
{
    return m_isCryptOpen && m_isMounted;
}

bool luks::canCryptOpen(const QString&) const
{
    return !m_isCryptOpen && !m_isMounted;
}

bool luks::canCryptClose(const QString&) const
{
    return m_isCryptOpen && !m_isMounted;
}

bool luks::isCryptOpen() const
{
    return m_isCryptOpen;
}

bool luks::cryptOpen(const QString& deviceNode)
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

    QPointer<DecryptLuksDialog> dlg = new DecryptLuksDialog(0, deviceNode); //TODO: parent widget instead of 0

    if (dlg->exec() != QDialog::Accepted)
    {
        delete dlg;
        return false;
    }

    std::vector<QString> commands;
    commands.push_back(QStringLiteral("echo"));
    commands.push_back(QStringLiteral("cryptsetup"));
    std::vector<QStringList> args;
    args.push_back(QStringList() << dlg->luksPassphrase().text());
    args.push_back(QStringList() << QStringLiteral("luksOpen") << deviceNode << dlg->luksName().text());
    delete dlg;

    ExternalCommand cmd(commands, args);
    if (!(cmd.run(-1) && cmd.exitCode() == 0))
        return false;

    if (m_innerFs)
    {
        delete m_innerFs;
        m_innerFs = nullptr;
    }

    QString mapperNode = mapperName(deviceNode);
    if (mapperNode.isEmpty())
        return false;

    FileSystem::Type innerFsType = detectFileSystem(mapperNode);
    m_innerFs = FileSystemFactory::cloneWithNewType(innerFsType,
                                                    *this);

    m_isCryptOpen = (m_innerFs != nullptr);

    if (m_isCryptOpen)
        return true;
    return false;
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

    ExternalCommand cmd(QStringLiteral("cryptsetup"), QStringList() << QStringLiteral("luksClose") << mapperName(deviceNode));
    if (!(cmd.run(-1) && cmd.exitCode() == 0))
        return false;

    delete m_innerFs;
    m_innerFs = nullptr;

    m_isCryptOpen = (m_innerFs != nullptr);

    if (!m_isCryptOpen)
        return true;
    return false;
}

bool luks::mount(const QString& deviceNode, const QString& mountPoint)
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

    if (m_innerFs->canMount(mapperNode))
    {
        if (m_innerFs->mount(mapperNode, mountPoint))
        {
            m_isMounted = true;
            return true;
        }
    }
    else {
        ExternalCommand mountCmd(
                QStringLiteral("mount"),
                { QStringLiteral("-v"), mapperNode, mountPoint });
        if (mountCmd.run() && mountCmd.exitCode() == 0)
        {
            m_isMounted = true;
            return true;
        }
    }
    return false;
}

bool luks::unmount(const QString& deviceNode)
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
        if (m_innerFs->unmount(mapperNode))
        {
            m_isMounted = false;
            return true;
        }
    }
    else {
        ExternalCommand unmountCmd(
                QStringLiteral("mount"),
                { QStringLiteral("-v"), QStringLiteral("-A"), mapperNode });
        if (unmountCmd.run() && unmountCmd.exitCode() == 0)
        {
            m_isMounted = false;
            return true;
        }
    }
    return false;
}

QString luks::readUUID(const QString& deviceNode) const
{
    ExternalCommand cmd(QStringLiteral("cryptsetup"), QStringList() << QStringLiteral("luksUUID") << deviceNode);
    if (cmd.run()) {
        return cmd.output().simplified();
    }
    return QStringLiteral("---");
}

bool luks::updateUUID(Report& report, const QString& deviceNode) const
{
    QUuid uuid = QUuid::createUuid();

    ExternalCommand cmd(report, QStringLiteral("cryptsetup"), QStringList() << QStringLiteral("luksUUID") << deviceNode << QStringLiteral("--uuid") << uuid.toString());
    return cmd.run(-1) && cmd.exitCode() == 0;
}

QString luks::mapperName(const QString& deviceNode)
{
    ExternalCommand cmd(QStringLiteral("find"), QStringList() << QStringLiteral("/dev/mapper/") << QStringLiteral("-exec") << QStringLiteral("cryptsetup") << QStringLiteral("status") << QStringLiteral("{}") << QStringLiteral(";"));
    if (cmd.run()) {
        QRegExp rxDeviceName(QStringLiteral("(/dev/mapper/[A-Za-z0-9-/]+) is active[A-Za-z0-9- \\.\n]+[A-Za-z0-9-: \n]+") + deviceNode);
        if (rxDeviceName.indexIn(cmd.output()) > -1)
            return rxDeviceName.cap(1);
    }
    return QString();
}

QString luks::getCipherName(const QString& deviceNode)
{
    ExternalCommand cmd(QStringLiteral("cryptsetup"), QStringList() << QStringLiteral("luksDump") << deviceNode);
    if (cmd.run()) {
        QRegExp rxCipherName(QStringLiteral("(?:Cipher name:\\s+)([A-Za-z0-9-]+)"));
        if (rxCipherName.indexIn(cmd.output()) > -1)
            return rxCipherName.cap(1);
    }
    return QStringLiteral("---");
}

QString luks::getCipherMode(const QString& deviceNode)
{
    ExternalCommand cmd(QStringLiteral("cryptsetup"), QStringList() << QStringLiteral("luksDump") << deviceNode);
    if (cmd.run()) {
        QRegExp rxCipherMode(QStringLiteral("(?:Cipher mode:\\s+)([A-Za-z0-9-]+)"));
        if (rxCipherMode.indexIn(cmd.output()) > -1)
            return rxCipherMode.cap(1);
    }
    return QStringLiteral("---");
}

QString luks::getHashName(const QString& deviceNode)
{
    ExternalCommand cmd(QStringLiteral("cryptsetup"), QStringList() << QStringLiteral("luksDump") << deviceNode);
    if (cmd.run()) {
        QRegExp rxHash(QStringLiteral("(?:Hash spec:\\s+)([A-Za-z0-9-]+)"));
        if (rxHash.indexIn(cmd.output()) > -1)
            return rxHash.cap(1);
    }
    return QStringLiteral("---");
}

QString luks::getKeySize(const QString& deviceNode)
{
    ExternalCommand cmd(QStringLiteral("cryptsetup"), QStringList() << QStringLiteral("luksDump") << deviceNode);
    if (cmd.run()) {
        QRegExp rxKeySize(QStringLiteral("(?:MK bits:\\s+)(\\d+)"));
        if (rxKeySize.indexIn(cmd.output()) > -1)
            return rxKeySize.cap(1);
    }
    return QStringLiteral("---");
}

QString luks::getPayloadOffset(const QString& deviceNode)
{
    ExternalCommand cmd(QStringLiteral("cryptsetup"), QStringList() << QStringLiteral("luksDump") << deviceNode);
    if (cmd.run()) {
        QRegExp rxPayloadOffset(QStringLiteral("(?:Payload offset:\\s+)(\\d+)"));
        if (rxPayloadOffset.indexIn(cmd.output()) > -1)
            return rxPayloadOffset.cap(1);
    }
    return QStringLiteral("---");
}
}
