/*
    SPDX-FileCopyrightText: 2008-2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2012-2018 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2015 Chris Campbell <c.j.campbell@ed.ac.uk>
    SPDX-FileCopyrightText: 2020 Arnaud Ferraris <arnaud.ferraris@collabora.com>
    SPDX-FileCopyrightText: 2020 Gaël PORTAY <gael.portay@collabora.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KPMCORE_EXT3_H
#define KPMCORE_EXT3_H

#include "util/libpartitionmanagerexport.h"

#include "fs/ext2.h"

#include <QtGlobal>

class Report;

class QString;

namespace FS
{
/** An ext3 file system.

    Basically the same as ext2.

    @author Volker Lanz <vl@fidra.de>
 */
class LIBKPMCORE_EXPORT ext3 : public ext2
{
public:
    ext3(qint64 firstsector, qint64 lastsector, qint64 sectorsused, const QString& label, const QVariantMap& features = {});

public:
    bool create(Report& report, const QString& deviceNode) override;
    bool resizeOnline(Report& report, const QString& deviceNode, const QString& mountPoint, qint64 length) const override;
    qint64 maxCapacity() const override;

    CommandSupportType supportGrowOnline() const override {
        return m_Grow;
    }

    QString posixPermissions() const override { return implPosixPermissions();  };
    void setPosixPermissions(const QString& permissions) override { implSetPosixPermissions(permissions); };
};
}

#endif
