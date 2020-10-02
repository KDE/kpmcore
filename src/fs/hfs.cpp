/*
    SPDX-FileCopyrightText: 2008-2011 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2013-2018 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2019 Yuri Chornoivan <yurchor@ukr.net>
    SPDX-FileCopyrightText: 2020 Arnaud Ferraris <arnaud.ferraris@collabora.com>
    SPDX-FileCopyrightText: 2020 Gaël PORTAY <gael.portay@collabora.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "fs/hfs.h"

#include "util/externalcommand.h"
#include "util/capacity.h"

#include <QStringList>

namespace FS
{
FileSystem::CommandSupportType hfs::m_GetUsed = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType hfs::m_GetLabel = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType hfs::m_Create = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType hfs::m_Shrink = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType hfs::m_Move = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType hfs::m_Check = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType hfs::m_Copy = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType hfs::m_Backup = FileSystem::cmdSupportNone;

hfs::hfs(qint64 firstsector, qint64 lastsector, qint64 sectorsused, const QString& label, const QVariantMap& features) :
    FileSystem(firstsector, lastsector, sectorsused, label, features, FileSystem::Type::Hfs)
{
}

void hfs::init()
{
    m_GetLabel = cmdSupportCore;
    m_Create = findExternal(QStringLiteral("hformat")) ? cmdSupportFileSystem : cmdSupportNone;
    m_Check = findExternal(QStringLiteral("hfsck")) ? cmdSupportFileSystem : cmdSupportNone;
    m_Move = m_Copy = (m_Check != cmdSupportNone) ? cmdSupportCore : cmdSupportNone;
    m_Backup = cmdSupportCore;
}

bool hfs::supportToolFound() const
{
    return
//          m_GetUsed != cmdSupportNone &&
        m_GetLabel != cmdSupportNone &&
//          m_SetLabel != cmdSupportNone &&
        m_Create != cmdSupportNone &&
        m_Check != cmdSupportNone &&
//          m_UpdateUUID != cmdSupportNone &&
//          m_Grow != cmdSupportNone &&
//          m_Shrink != cmdSupportNone &&
        m_Copy != cmdSupportNone &&
        m_Move != cmdSupportNone &&
        m_Backup != cmdSupportNone;
//          m_GetUUID != cmdSupportNone;
}

FileSystem::SupportTool hfs::supportToolName() const
{
    return SupportTool(QStringLiteral("hfsutils"), QUrl(QStringLiteral("https://www.mars.org/home/rob/proj/hfs/")));
}


qint64 hfs::maxCapacity() const
{
    return 2 * Capacity::unitFactor(Capacity::Unit::Byte, Capacity::Unit::TiB);
}

int hfs::maxLabelLength() const
{
    return 27;
}

bool hfs::check(Report& report, const QString& deviceNode) const
{
    ExternalCommand cmd(report, QStringLiteral("hfsck"), { QStringLiteral("-v"), deviceNode });
    return cmd.run(-1) && cmd.exitCode() == 0;
}

bool hfs::create(Report& report, const QString& deviceNode)
{
    ExternalCommand cmd(report, QStringLiteral("hformat"), { deviceNode });
    return cmd.run(-1) && cmd.exitCode() == 0;
}
}
