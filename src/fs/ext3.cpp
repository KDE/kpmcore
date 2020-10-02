/*
    SPDX-FileCopyrightText: 2008-2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2012-2018 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2020 Arnaud Ferraris <arnaud.ferraris@collabora.com>
    SPDX-FileCopyrightText: 2020 Gaël PORTAY <gael.portay@collabora.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "fs/ext3.h"

#include "util/externalcommand.h"
#include "util/capacity.h"

#include <QStringList>

namespace FS
{
ext3::ext3(qint64 firstsector, qint64 lastsector, qint64 sectorsused, const QString& label, const QVariantMap& features) :
    ext2(firstsector, lastsector, sectorsused, label, features, FileSystem::Type::Ext3)
{
}

qint64 ext3::maxCapacity() const
{
    return 16 * Capacity::unitFactor(Capacity::Unit::Byte, Capacity::Unit::TiB) - Capacity::unitFactor(Capacity::Unit::Byte, Capacity::Unit::MiB);
}

bool ext3::create(Report& report, const QString& deviceNode)
{
    QStringList args = QStringList();

    if (!this->features().isEmpty()) {
        QStringList feature_list = QStringList();
        for (const auto& k : this->features().keys()) {
            const auto& v = this->features().value(k);
            if (v.type() == QVariant::Type::Bool) {
                if (v.toBool())
                    feature_list << k;
		else
                    feature_list << (QStringLiteral("^") +  k);
            } else {
                qWarning() << "Ignoring feature" << k << "of type" << v.type() << "; requires type QVariant::bool.";
            }
        }
        args << QStringLiteral("-O") << feature_list.join(QStringLiteral(","));
    }
    args << QStringLiteral("-qF") << deviceNode;

    ExternalCommand cmd(report, QStringLiteral("mkfs.ext3"), args);
    return cmd.run(-1) && cmd.exitCode() == 0;
}

bool ext3::resizeOnline(Report& report, const QString& deviceNode, const QString&, qint64 length) const
{
    return resize(report, deviceNode, length);
}
}
