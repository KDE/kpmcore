/*************************************************************************
  *  Copyright (C) 2020 by Gaël PORTAY <gael.portay@collabora.com>       *
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

#include "plugins/sfdisk/sfdiskgptattributes.h"

#include <QString>
#include <QStringList>

const static QString requiredPartition = QStringLiteral("RequiredPartition");
const static QString noBlockIoProtocol = QStringLiteral("NoBlockIOProtocol");
const static QString legacyBiosBootable = QStringLiteral("LegacyBIOSBootable");
const static QString guid = QStringLiteral("GUID:");

quint64 SfdiskGptAttributes::toULongLong(const QStringList& attrs)
{
    quint64 attributes = 0;
    for (auto& attr: attrs)
        if (attr.compare(requiredPartition) == 0)
            attributes |= 0x1ULL;
        else if (attr.compare(noBlockIoProtocol) == 0)
            attributes |= 0x2ULL;
        else if (attr.compare(legacyBiosBootable) == 0)
            attributes |= 0x4ULL;
        else if (attr.startsWith(guid))
            attributes |= 1ULL << QStringRef(&attr, guid.length(), attr.length() - guid.length()).toULongLong();

    return attributes;
}

QStringList SfdiskGptAttributes::toStringList(quint64 attrs)
{
    QStringList list;
    if (attrs & 0x1)
        list += requiredPartition;
    if (attrs & 0x2)
        list += noBlockIoProtocol;
    if (attrs & 0x4)
        list += legacyBiosBootable;
    for (int bit = 48; bit < 64; bit++)
        if (attrs & (1 << bit))
            list += guid + QString::number(bit);

    return list;
}
