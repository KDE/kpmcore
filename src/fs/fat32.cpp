/*************************************************************************
 *  Copyright (C) 2008 by Volker Lanz <vl@fidra.de>                      *
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

#include "fs/fat32.h"

#include "util/externalcommand.h"
#include "util/capacity.h"

#include <QStringList>

#include <ctime>

namespace FS
{
fat32::fat32(qint64 firstsector, qint64 lastsector, qint64 sectorsused, const QString& label) :
    fat16(firstsector, lastsector, sectorsused, label, FileSystem::Type::Fat32)
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
    ExternalCommand cmd(report, QStringLiteral("mkfs.fat"), { QStringLiteral("-F32"), QStringLiteral("-I"), QStringLiteral("-v"), deviceNode });
    return cmd.run(-1) && cmd.exitCode() == 0;
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
