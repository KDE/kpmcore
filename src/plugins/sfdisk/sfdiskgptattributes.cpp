/*
    SPDX-FileCopyrightText: 2020 Gaël PORTAY <gael.portay@collabora.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "plugins/sfdisk/sfdiskgptattributes.h"

#include <QString>
#include <QStringList>
#include <QStringView>

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
            attributes |= 1ULL << QStringView{ attr }.mid(guid.length(), attr.length() - guid.length()).toULongLong();

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
        if (attrs & (1LL << bit))
            list += guid + QString::number(bit);

    return list;
}
