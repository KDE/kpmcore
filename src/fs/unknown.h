/*
    SPDX-FileCopyrightText: 2008-2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2015 Chris Campbell <c.j.campbell@ed.ac.uk>
    SPDX-FileCopyrightText: 2016-2019 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2020 Arnaud Ferraris <arnaud.ferraris@collabora.com>
    SPDX-FileCopyrightText: 2020 Gaël PORTAY <gael.portay@collabora.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KPMCORE_UNKNOWN_H
#define KPMCORE_UNKNOWN_H

#include "util/libpartitionmanagerexport.h"

#include "fs/filesystem.h"

#include <QtGlobal>

namespace FS
{
/** A pseudo file system for partitions whose file system we cannot determine.
    @author Volker Lanz <vl@fidra.de>
*/
class LIBKPMCORE_EXPORT unknown : public FileSystem
{
public:
    unknown(qint64 firstsector, qint64 lastsector, qint64 sectorsused, const QString& label, const QVariantMap& features = {});

public:
    bool supportToolFound() const override {
        return true;
    }
    bool canMount(const QString & deviceNode, const QString & mountPoint) const override;

    CommandSupportType supportMove() const override {
        return m_Move;
    }

    static CommandSupportType m_Move;
};
}

#endif
