/*
    SPDX-FileCopyrightText: 2008-2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2015 Chris Campbell <c.j.campbell@ed.ac.uk>
    SPDX-FileCopyrightText: 2016 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2020 Arnaud Ferraris <arnaud.ferraris@collabora.com>
    SPDX-FileCopyrightText: 2020 Gaël PORTAY <gael.portay@collabora.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KPMCORE_UNFORMATTED_H
#define KPMCORE_UNFORMATTED_H

#include "util/libpartitionmanagerexport.h"

#include "fs/filesystem.h"

#include <QtGlobal>

class Report;

class QString;

namespace FS
{
/** A pseudo file system for unformatted partitions.
    @author Volker Lanz <vl@fidra.de>
 */
class LIBKPMCORE_EXPORT unformatted : public FileSystem
{
public:
    unformatted(qint64 firstsector, qint64 lastsector, qint64 sectorsused, const QString& label, const QVariantMap& features = {});

public:
    bool create(Report&, const QString&) override;

    CommandSupportType supportCreate() const override {
        return m_Create;
    }

    bool supportToolFound() const override {
        return true;
    }

public:
    static CommandSupportType m_Create;
};
}

#endif
