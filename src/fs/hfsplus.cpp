/*
    SPDX-FileCopyrightText: 2008-2011 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2013-2018 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2019 Yuri Chornoivan <yurchor@ukr.net>
    SPDX-FileCopyrightText: 2020 Arnaud Ferraris <arnaud.ferraris@collabora.com>
    SPDX-FileCopyrightText: 2020 Gaël PORTAY <gael.portay@collabora.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "fs/hfsplus.h"

#include "util/externalcommand.h"
#include "util/capacity.h"

#include <QStringList>

namespace FS
{
FileSystem::CommandSupportType hfsplus::m_GetLabel = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType hfsplus::m_GetUsed = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType hfsplus::m_Shrink = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType hfsplus::m_Move = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType hfsplus::m_Check = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType hfsplus::m_Create = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType hfsplus::m_Copy = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType hfsplus::m_Backup = FileSystem::cmdSupportNone;

hfsplus::hfsplus(qint64 firstsector, qint64 lastsector, qint64 sectorsused, const QString& label, const QVariantMap& features) :
    FileSystem(firstsector, lastsector, sectorsused, label, features, FileSystem::Type::HfsPlus)
{
}

void hfsplus::init()
{
    m_Check = findExternal(QStringLiteral("fsck.hfsplus")) ? cmdSupportFileSystem : cmdSupportNone;
    m_Create = findExternal(QStringLiteral("mkfs.hfsplus")) ? cmdSupportFileSystem : cmdSupportNone;
    m_Copy = (m_Check != cmdSupportNone) ? cmdSupportCore : cmdSupportNone;
    m_Move = (m_Check != cmdSupportNone) ? cmdSupportCore : cmdSupportNone;
    m_Backup = cmdSupportCore;
    m_GetLabel = cmdSupportCore;
}

bool hfsplus::supportToolFound() const
{
    return
//          m_GetUsed != cmdSupportNone &&
        m_GetLabel != cmdSupportNone &&
//          m_SetLabel != cmdSupportNone &&
        m_Create != cmdSupportNone &&
        m_Check != cmdSupportNone &&
//          m_UpdateUUID != cmdSupportNone &&
//          m_Grow != cmdSupportNone &&
        m_Shrink != cmdSupportNone &&
        m_Copy != cmdSupportNone &&
        m_Move != cmdSupportNone &&
        m_Backup != cmdSupportNone;
//         m_GetUUID != cmdSupportNone;
}

FileSystem::SupportTool hfsplus::supportToolName() const
{
    return SupportTool(QStringLiteral("diskdev_cmds"), QUrl(QStringLiteral("https://opensource.apple.com/tarballs/diskdev_cmds/")));
}

qint64 hfsplus::maxCapacity() const
{
    return Capacity::unitFactor(Capacity::Unit::Byte, Capacity::Unit::EiB);
}

int hfsplus::maxLabelLength() const
{
    return 63;
}

bool hfsplus::check(Report& report, const QString& deviceNode) const
{
    ExternalCommand cmd(report, QStringLiteral("fsck.hfsplus"), { deviceNode });
    return cmd.run(-1) && cmd.exitCode() == 0;
}

bool hfsplus::create(Report& report, const QString& deviceNode)
{
    ExternalCommand cmd(report, QStringLiteral("mkfs.hfsplus"), { deviceNode });
    return cmd.run(-1) && cmd.exitCode() == 0;
}

}
