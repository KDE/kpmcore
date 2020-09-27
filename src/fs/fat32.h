/*
    SPDX-FileCopyrightText: 2008-2011 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2008 Laurent Montel <montel@kde.org>
    SPDX-FileCopyrightText: 2013-2017 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2015 Chris Campbell <c.j.campbell@ed.ac.uk>
    SPDX-FileCopyrightText: 2015 Teo Mrnjavac <teo@kde.org>
    SPDX-FileCopyrightText: 2020 Arnaud Ferraris <arnaud.ferraris@collabora.com>
    SPDX-FileCopyrightText: 2020 Gaël PORTAY <gael.portay@collabora.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KPMCORE_FAT32_H
#define KPMCORE_FAT32_H

#include "util/libpartitionmanagerexport.h"

#include "fs/fat16.h"

#include <QtGlobal>

class Report;

class QString;

namespace FS
{
/** A fat32 file system.

    Basically the same as a fat16 file system.

    @author Volker Lanz <vl@fidra.de>
 */
class LIBKPMCORE_EXPORT fat32 : public fat16
{
public:
    fat32(qint64 firstsector, qint64 lastsector, qint64 sectorsused, const QString& label, const QVariantMap& features = {});

public:
    bool create(Report& report, const QString& deviceNode) override;
    bool updateUUID(Report& report, const QString& deviceNode) const override;

    qint64 minCapacity() const override;
    qint64 maxCapacity() const override;
};
}

#endif
