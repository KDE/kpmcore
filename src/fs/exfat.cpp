/*
    SPDX-FileCopyrightText: 2012-2018 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2019 Yuri Chornoivan <yurchor@ukr.net>
    SPDX-FileCopyrightText: 2020 Arnaud Ferraris <arnaud.ferraris@collabora.com>
    SPDX-FileCopyrightText: 2020 Gaël PORTAY <gael.portay@collabora.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "fs/exfat.h"

#include "util/externalcommand.h"
#include "util/capacity.h"

#include <QString>

namespace FS
{
FileSystem::CommandSupportType exfat::m_GetUsed = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType exfat::m_GetLabel = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType exfat::m_Create = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType exfat::m_Grow = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType exfat::m_Shrink = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType exfat::m_Move = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType exfat::m_Check = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType exfat::m_Copy = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType exfat::m_Backup = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType exfat::m_SetLabel = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType exfat::m_UpdateUUID = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType exfat::m_GetUUID = FileSystem::cmdSupportNone;
bool exfat::exfatUtils = false;

exfat::exfat(qint64 firstsector, qint64 lastsector, qint64 sectorsused, const QString& label, const QVariantMap& features) :
    FileSystem(firstsector, lastsector, sectorsused, label, features, FileSystem::Type::Exfat)
{
}

void exfat::init()
{
    // Check if we are using exfat-utils or exfatprogs
    exfatUtils = findExternal(QStringLiteral("mkexfatfs"));
    if (exfatUtils) {
        m_Create = cmdSupportFileSystem;
        m_Check = findExternal(QStringLiteral("fsck.exfat"), {}, 1) ? cmdSupportFileSystem : cmdSupportNone;
        m_SetLabel = findExternal(QStringLiteral("exfatlabel")) ? cmdSupportFileSystem : cmdSupportNone;
    }
    else {
        m_Create = findExternal(QStringLiteral("mkfs.exfat"), {}, 1) ? cmdSupportFileSystem : cmdSupportNone;
        m_Check = findExternal(QStringLiteral("fsck.exfat"), {}, 16) ? cmdSupportFileSystem : cmdSupportNone;
        m_SetLabel = findExternal(QStringLiteral("tune.exfat")) ? cmdSupportFileSystem : cmdSupportNone;
    }

    m_GetLabel = cmdSupportCore;
    m_UpdateUUID = cmdSupportNone;

    m_Copy = (m_Check != cmdSupportNone) ? cmdSupportCore : cmdSupportNone;
    m_Move = (m_Check != cmdSupportNone) ? cmdSupportCore : cmdSupportNone;

    m_GetLabel = cmdSupportCore;
    m_Backup = cmdSupportCore;
    m_GetUUID = cmdSupportCore;
}

bool exfat::supportToolFound() const
{
    return
//          m_GetUsed != cmdSupportNone &&
        m_GetLabel != cmdSupportNone &&
        m_SetLabel != cmdSupportNone &&
        m_Create != cmdSupportNone &&
        m_Check != cmdSupportNone &&
//          m_UpdateUUID != cmdSupportNone &&
//          m_Grow != cmdSupportNone &&
//          m_Shrink != cmdSupportNone &&
        m_Copy != cmdSupportNone &&
        m_Move != cmdSupportNone &&
        m_Backup != cmdSupportNone &&
        m_GetUUID != cmdSupportNone;
}

FileSystem::SupportTool exfat::supportToolName() const
{
    return SupportTool(QStringLiteral("exfatprogs"), QUrl(QStringLiteral("https://github.com/exfatprogs/exfatprogs")));
}

qint64 exfat::maxCapacity() const
{
    return Capacity::unitFactor(Capacity::Unit::Byte, Capacity::Unit::EiB);
}

int exfat::maxLabelLength() const
{
    return 11;
}

bool exfat::check(Report& report, const QString& deviceNode) const
{
    ExternalCommand cmd(report, QStringLiteral("fsck.exfat"), { QStringLiteral("--repair-yes"), QStringLiteral("--verbose"), deviceNode });
    return cmd.run(-1) && cmd.exitCode() == 0;
}

bool exfat::create(Report& report, const QString& deviceNode)
{
    ExternalCommand cmd(report, QStringLiteral("mkfs.exfat"), { deviceNode });
    return cmd.run(-1) && cmd.exitCode() == 0;
}

bool exfat::writeLabel(Report& report, const QString& deviceNode, const QString& newLabel)
{
    ExternalCommand cmd(report);
    if (exfatUtils) {
        cmd.setCommand(QStringLiteral("exfatlabel"));
        cmd.setArgs({ deviceNode, newLabel });
    }
    else {
        cmd.setCommand(QStringLiteral("tune.exfat"));
        cmd.setArgs({ deviceNode, QStringLiteral("-L"), newLabel });
    }

    return cmd.run(-1) && cmd.exitCode() == 0;
}

bool exfat::updateUUID(Report& report, const QString& deviceNode) const
{
    Q_UNUSED(report)
    Q_UNUSED(deviceNode)

    return false;
}
}
