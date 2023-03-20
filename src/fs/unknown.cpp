/*
    SPDX-FileCopyrightText: 2008 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2016-2019 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2020 Arnaud Ferraris <arnaud.ferraris@collabora.com>
    SPDX-FileCopyrightText: 2020 Gaël PORTAY <gael.portay@collabora.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "fs/unknown.h"

namespace FS
{

FileSystem::CommandSupportType unknown::m_Move = FileSystem::cmdSupportCore;
FileSystem::CommandSupportType unknown::m_Copy = FileSystem::cmdSupportCore;

unknown::unknown(qint64 firstsector, qint64 lastsector, qint64 sectorsused, const QString& label, const QVariantMap& features) :
    FileSystem(firstsector, lastsector, sectorsused, label, features, FileSystem::Type::Unknown)
{
}

bool unknown::canMount(const QString & deviceNode, const QString & mountPoint) const
{
    Q_UNUSED(deviceNode)
    Q_UNUSED(mountPoint)
    return false;
}
}
