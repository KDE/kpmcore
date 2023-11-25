/*
    SPDX-FileCopyrightText: 2023 Er2 <er2@dismail.de>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "fs/freebsdswap.h"

#include "util/externalcommand.h"

#include <KLocalizedString>

#include <QFileInfo>
#include <QRegularExpression>
#include <QTextStream>

namespace FS
{
FileSystem::CommandSupportType freebsdswap::m_Create = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType freebsdswap::m_Grow = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType freebsdswap::m_Shrink = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType freebsdswap::m_Move = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType freebsdswap::m_Copy = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType freebsdswap::m_GetUsed = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType freebsdswap::m_GetLabel = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType freebsdswap::m_SetLabel = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType freebsdswap::m_GetUUID = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType freebsdswap::m_UpdateUUID = FileSystem::cmdSupportNone;

freebsdswap::freebsdswap(qint64 firstsector, qint64 lastsector, qint64 sectorsused, const QString& label, const QVariantMap& features) :
    FileSystem(firstsector, lastsector, sectorsused, label, features, FileSystem::Type::FreeBSDSwap)
{
}

void freebsdswap::init()
{
    m_Create = findExternal(QStringLiteral("swapctl")) ? cmdSupportFileSystem : cmdSupportNone;
    m_Shrink = cmdSupportFileSystem;
    m_Grow = cmdSupportFileSystem;
    m_SetLabel = cmdSupportNone;
    m_UpdateUUID = cmdSupportNone;
    m_GetLabel = cmdSupportCore;
    m_GetUsed = cmdSupportFileSystem;
    m_Copy = cmdSupportNone;
    m_Move = cmdSupportCore;
    m_GetUUID = cmdSupportNone;
}

bool freebsdswap::supportToolFound() const
{
    // UUID and labels are always unsupported by FreeBSD swap
    // everything other should be not cmdSupportNone
    return
        m_GetUsed != cmdSupportNone &&
        m_GetLabel != cmdSupportNone &&
        m_SetLabel != cmdSupportNone &&
        m_Create != cmdSupportNone &&
//         m_Check != cmdSupportNone &&
//         m_UpdateUUID != cmdSupportNone &&
//         m_Grow != cmdSupportNone &&
//         m_Shrink != cmdSupportNone &&
//         m_Copy != cmdSupportNone &&
        m_Move != cmdSupportNone; // &&
//         m_Backup != cmdSupportNone &&
//         m_GetUUID != cmdSupportNone;
}

bool freebsdswap::create(Report&, const QString&)
{
    // FreeBSD swap doesn't need to be prepared

    return true;
}

QString freebsdswap::mountTitle() const
{
    return xi18nc("@title:menu", "Activate swap");
}

QString freebsdswap::unmountTitle() const
{
    return xi18nc("@title:menu", "Deactivate swap");
}

bool freebsdswap::canMount(const QString&, const QString& mountPoint) const {
    // FreeBSD swap doesn't need mount point to activate
    return mountPoint != QStringLiteral("/");
}

bool freebsdswap::mount(Report& report, const QString& deviceNode, const QString&)
{
    // On FreeBSD, swapctl, swapoff and swapon are hardlinks to the same executable

    ExternalCommand cmd(report, QStringLiteral("swapon"), { deviceNode });
    return cmd.run(-1) && cmd.exitCode() == 0;
}

bool freebsdswap::unmount(Report& report, const QString& deviceNode)
{
    ExternalCommand cmd(report, QStringLiteral("swapoff"), { deviceNode });
    return cmd.run(-1) && cmd.exitCode() == 0;
}

qint64 freebsdswap::readUsedCapacity(const QString& deviceNode) const
{
    QFileInfo kernelPath(deviceNode);
    ExternalCommand cmd(QStringLiteral("swapctl"), { QStringLiteral("-l") });

    if (cmd.run(-1) && cmd.exitCode() == 0) {
        QByteArray data = cmd.rawOutput();

        /* parse swapctl output
         * Device:       1024-blocks     Used:
         * /dev/ada0p3     2097152         0
         */
        QTextStream in(&data);
        while (!in.atEnd()) {
            QStringList line = in.readLine().split(QRegularExpression(QStringLiteral("\\s+")));
            if (line[0] == kernelPath.canonicalFilePath()) {
                return line[1].toLongLong() * 1024;
            }
        }
    }
    return -1;
}
}
