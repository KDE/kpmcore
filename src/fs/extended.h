/*
    SPDX-FileCopyrightText: 2008-2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2015 Chris Campbell <c.j.campbell@ed.ac.uk>
    SPDX-FileCopyrightText: 2016-2018 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2020 Arnaud Ferraris <arnaud.ferraris@collabora.com>
    SPDX-FileCopyrightText: 2020 Gaël PORTAY <gael.portay@collabora.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KPMCORE_EXTENDED_H
#define KPMCORE_EXTENDED_H

#include "util/libpartitionmanagerexport.h"

#include "fs/filesystem.h"

#include <QtGlobal>

class QString;

namespace FS
{
/** An extended file system.

    A FileSystem for an extended Partition. Of course, extended partitions do not actually have
    a file system, but we need this to be able to create, grow, shrink or move them.

    @author Volker Lanz <vl@fidra.de>
 */

class LIBKPMCORE_EXPORT extended : public FileSystem
{
public:
    extended(qint64 firstsector, qint64 lastsector, qint64 sectorsused, const QString& label, const QVariantMap& features = {});

public:

    bool create(Report&, const QString&) override;

    CommandSupportType supportCreate() const override {
        return m_Create;
    }
    CommandSupportType supportGrow() const override {
        return m_Grow;
    }
    CommandSupportType supportShrink() const override {
        return m_Shrink;
    }
    CommandSupportType supportMove() const override {
        return m_Move;
    }

    bool supportToolFound() const override {
        return true;
    }

public:
    static CommandSupportType m_Create;
    static CommandSupportType m_Grow;
    static CommandSupportType m_Shrink;
    static CommandSupportType m_Move;
};
}

#endif
