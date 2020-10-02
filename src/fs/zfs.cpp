/*
    SPDX-FileCopyrightText: 2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2012-2018 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2019 Yuri Chornoivan <yurchor@ukr.net>
    SPDX-FileCopyrightText: 2020 Arnaud Ferraris <arnaud.ferraris@collabora.com>
    SPDX-FileCopyrightText: 2020 Gaël PORTAY <gael.portay@collabora.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "fs/zfs.h"

#include "util/externalcommand.h"
#include "util/capacity.h"
#include "util/report.h"

#include <QString>

namespace FS
{
FileSystem::CommandSupportType zfs::m_GetUsed = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType zfs::m_GetLabel = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType zfs::m_Create = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType zfs::m_Grow = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType zfs::m_Shrink = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType zfs::m_Move = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType zfs::m_Check = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType zfs::m_Copy = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType zfs::m_Backup = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType zfs::m_SetLabel = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType zfs::m_UpdateUUID = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType zfs::m_GetUUID = FileSystem::cmdSupportNone;

zfs::zfs(qint64 firstsector, qint64 lastsector, qint64 sectorsused, const QString& label, const QVariantMap& features) :
    FileSystem(firstsector, lastsector, sectorsused, label, features, FileSystem::Type::Zfs)
{
}

void zfs::init()
{
    m_SetLabel = findExternal(QStringLiteral("zpool"), {}, 2) ? cmdSupportFileSystem : cmdSupportNone;

    m_GetLabel = cmdSupportCore;
    m_Backup = cmdSupportCore;
    m_GetUUID = cmdSupportCore;
}

bool zfs::supportToolFound() const
{
    return
//          m_GetUsed != cmdSupportNone &&
        m_GetLabel != cmdSupportNone &&
        m_SetLabel != cmdSupportNone &&
//          m_Create != cmdSupportNone &&
//          m_Check != cmdSupportNone &&
//          m_UpdateUUID != cmdSupportNone &&
//          m_Grow != cmdSupportNone &&
//          m_Shrink != cmdSupportNone &&
//          m_Copy != cmdSupportNone &&
//          m_Move != cmdSupportNone &&
        m_Backup != cmdSupportNone &&
        m_GetUUID != cmdSupportNone;
}

FileSystem::SupportTool zfs::supportToolName() const
{
    return SupportTool(QStringLiteral("zfs"), QUrl(QStringLiteral("https://zfsonlinux.org/")));
}

qint64 zfs::minCapacity() const
{
    return 64 * Capacity::unitFactor(Capacity::Unit::Byte, Capacity::Unit::MiB);
}

qint64 zfs::maxCapacity() const
{
    return Capacity::unitFactor(Capacity::Unit::Byte, Capacity::Unit::EiB);
}

bool zfs::remove(Report& report, const QString& deviceNode) const
{
    Q_UNUSED(deviceNode)
    ExternalCommand cmd(report, QStringLiteral("zpool"), { QStringLiteral("destroy"), QStringLiteral("-f"), label() });
    return cmd.run(-1) && cmd.exitCode() == 0;
}

bool zfs::writeLabel(Report& report, const QString& deviceNode, const QString& newLabel)
{
    Q_UNUSED(deviceNode)
    ExternalCommand cmd1(report, QStringLiteral("zpool"), { QStringLiteral("export"), label() });
    ExternalCommand cmd2(report, QStringLiteral("zpool"), { QStringLiteral("import"), label(), newLabel });
    return cmd1.run(-1) && cmd1.exitCode() == 0 && cmd2.run(-1) && cmd2.exitCode() == 0;
}
}
