/*
    SPDX-FileCopyrightText: 2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2016-2018 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2020 Arnaud Ferraris <arnaud.ferraris@collabora.com>
    SPDX-FileCopyrightText: 2020 Gaël PORTAY <gael.portay@collabora.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "fs/hpfs.h"

#include "util/capacity.h"

#include <QString>

namespace FS
{
FileSystem::CommandSupportType hpfs::m_GetUsed = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType hpfs::m_GetLabel = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType hpfs::m_Create = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType hpfs::m_Grow = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType hpfs::m_Shrink = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType hpfs::m_Move = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType hpfs::m_Check = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType hpfs::m_Copy = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType hpfs::m_Backup = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType hpfs::m_SetLabel = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType hpfs::m_UpdateUUID = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType hpfs::m_GetUUID = FileSystem::cmdSupportNone;

hpfs::hpfs(qint64 firstsector, qint64 lastsector, qint64 sectorsused, const QString& label, const QVariantMap& features) :
    FileSystem(firstsector, lastsector, sectorsused, label, features, FileSystem::Type::Hpfs)
{
}

qint64 hpfs::maxCapacity() const
{
    return 2 * Capacity::unitFactor(Capacity::Unit::Byte, Capacity::Unit::TiB);
}
}
