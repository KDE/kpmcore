/*
    SPDX-FileCopyrightText: 2008-2011 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2008 Laurent Montel <montel@kde.org>
    SPDX-FileCopyrightText: 2013-2019 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2015 Chris Campbell <c.j.campbell@ed.ac.uk>
    SPDX-FileCopyrightText: 2015 Teo Mrnjavac <teo@kde.org>
    SPDX-FileCopyrightText: 2020 Arnaud Ferraris <arnaud.ferraris@collabora.com>
    SPDX-FileCopyrightText: 2020 Gaël PORTAY <gael.portay@collabora.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KPMCORE_FAT16_H
#define KPMCORE_FAT16_H

#include "fs/fat12.h"

class Report;

class QString;

namespace FS
{
/** A fat16 file system.
    @author Volker Lanz <vl@fidra.de>
 */
class LIBKPMCORE_EXPORT fat16 : public fat12
{
public:
    fat16(qint64 firstsector, qint64 lastsector, qint64 sectorsused, const QString& label, const QVariantMap& features = {}, FileSystem::Type type = FileSystem::Type::Fat16);

public:
    void init() override;

    bool create(Report& report, const QString& deviceNode) override;
    bool resize(Report& report, const QString& deviceNode, qint64 length) const override;

    CommandSupportType supportGrow() const override {
        return m_Grow;
    }
    CommandSupportType supportShrink() const override {
        return m_Shrink;
    }

    qint64 minCapacity() const override;
    qint64 maxCapacity() const override;
    bool supportToolFound() const override;
};
}

#endif
