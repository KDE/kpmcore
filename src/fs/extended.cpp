/*
    SPDX-FileCopyrightText: 2008-2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2016-2018 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2020 Arnaud Ferraris <arnaud.ferraris@collabora.com>
    SPDX-FileCopyrightText: 2020 Gaël PORTAY <gael.portay@collabora.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "fs/extended.h"

namespace FS
{
FileSystem::CommandSupportType extended::m_Create = FileSystem::cmdSupportFileSystem;
FileSystem::CommandSupportType extended::m_Grow = FileSystem::cmdSupportCore;
FileSystem::CommandSupportType extended::m_Shrink = FileSystem::cmdSupportCore;
FileSystem::CommandSupportType extended::m_Move = FileSystem::cmdSupportCore;

extended::extended(qint64 firstsector, qint64 lastsector, qint64 sectorsused, const QString& label, const QVariantMap& features) :
    FileSystem(firstsector, lastsector, sectorsused, label, features, FileSystem::Type::Extended)
{
}

bool extended::create(Report&, const QString&)
{
    return true;
}
}

