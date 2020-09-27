/*
    SPDX-FileCopyrightText: 2008-2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2013-2017 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2015 Chris Campbell <c.j.campbell@ed.ac.uk>
    SPDX-FileCopyrightText: 2020 Arnaud Ferraris <arnaud.ferraris@collabora.com>
    SPDX-FileCopyrightText: 2020 Gaël PORTAY <gael.portay@collabora.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KPMCORE_HFS_H
#define KPMCORE_HFS_H

#include "util/libpartitionmanagerexport.h"

#include "fs/filesystem.h"

#include <QtGlobal>

class Report;

class QString;

namespace FS
{
/** An hfs file system.
    @author Volker Lanz <vl@fidra.de>
 */
class LIBKPMCORE_EXPORT hfs : public FileSystem
{
public:
    hfs(qint64 firstsector, qint64 lastsector, qint64 sectorsused, const QString& label, const QVariantMap& features = {});

public:
    void init() override;

    bool check(Report& report, const QString& deviceNode) const override;
    bool create(Report& report, const QString& deviceNode) override;

    CommandSupportType supportGetUsed() const override {
        return m_GetUsed;
    }
    CommandSupportType supportGetLabel() const override {
        return m_GetLabel;
    }
    CommandSupportType supportCreate() const override {
        return m_Create;
    }
    CommandSupportType supportShrink() const override {
        return m_Shrink;
    }
    CommandSupportType supportMove() const override {
        return m_Move;
    }
    CommandSupportType supportCheck() const override {
        return m_Check;
    }
    CommandSupportType supportCopy() const override {
        return m_Copy;
    }
    CommandSupportType supportBackup() const override {
        return m_Backup;
    }

    qint64 maxCapacity() const override;
    int maxLabelLength() const override;
    SupportTool supportToolName() const override;
    bool supportToolFound() const override;

public:
    static CommandSupportType m_GetUsed;
    static CommandSupportType m_GetLabel;
    static CommandSupportType m_Create;
    static CommandSupportType m_Shrink;
    static CommandSupportType m_Move;
    static CommandSupportType m_Check;
    static CommandSupportType m_Copy;
    static CommandSupportType m_Backup;
};
}

#endif
