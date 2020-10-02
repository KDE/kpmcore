/*
    SPDX-FileCopyrightText: 2008-2011 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2013-2018 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2015 Teo Mrnjavac <teo@kde.org>
    SPDX-FileCopyrightText: 2020 Arnaud Ferraris <arnaud.ferraris@collabora.com>
    SPDX-FileCopyrightText: 2020 Gaël PORTAY <gael.portay@collabora.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "fs/fat32.h"

#include "util/externalcommand.h"
#include "util/capacity.h"

#include <QStringList>

#include <ctime>

namespace FS
{
fat32::fat32(qint64 firstsector, qint64 lastsector, qint64 sectorsused, const QString& label, const QVariantMap& features) :
    fat16(firstsector, lastsector, sectorsused, label, features, FileSystem::Type::Fat32)
{
}

qint64 fat32::minCapacity() const
{
    return 32 * Capacity::unitFactor(Capacity::Unit::Byte, Capacity::Unit::MiB);
}

qint64 fat32::maxCapacity() const
{
    return 16 * Capacity::unitFactor(Capacity::Unit::Byte, Capacity::Unit::TiB) - Capacity::unitFactor(Capacity::Unit::Byte, Capacity::Unit::MiB);
}

bool fat32::create(Report& report, const QString& deviceNode)
{
    return createWithFatSize(report, deviceNode, 32);
}

bool fat32::updateUUID(Report& report, const QString& deviceNode) const
{
    // HACK: replace this hack with fatlabel "-i" (dosfstools 4.2)
    long int t = time(nullptr);

    char uuid[4];
    for (auto &u : uuid) {
        u = static_cast<char>(t & 0xff);
        t >>= 8;
    }

    ExternalCommand cmd;
    return cmd.writeData(report, QByteArray(uuid, sizeof(uuid)), deviceNode, 67);
}
}
