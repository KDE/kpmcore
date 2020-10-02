/*
    SPDX-FileCopyrightText: 2016-2018 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2020 Arnaud Ferraris <arnaud.ferraris@collabora.com>
    SPDX-FileCopyrightText: 2020 Gaël PORTAY <gael.portay@collabora.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KPMCORE_F2FS_H
#define KPMCORE_F2FS_H

#include "util/libpartitionmanagerexport.h"

#include "fs/filesystem.h"

#include <QtGlobal>

class Report;

class QString;

namespace FS
{
/** A f2fs file system.
    @author Andrius Štikonas <andrius@stikonas.eu>
*/
class LIBKPMCORE_EXPORT f2fs : public FileSystem
{
public:
    f2fs(qint64 firstsector, qint64 lastsector, qint64 sectorsused, const QString& label, const QVariantMap& features = {});

public:
    void init() override;

//          qint64 readUsedCapacity(const QString& deviceNode) const override;
    bool check(Report& report, const QString& deviceNode) const override;
    bool create(Report& report, const QString& deviceNode) override;
    bool createWithLabel(Report& report, const QString& deviceNode, const QString& label) override;
//     qint64 readUsedCapacity(const QString& deviceNode) const override;
    bool resize(Report& report, const QString& deviceNode, qint64 length) const override;
//     bool writeLabel(Report& report, const QString& deviceNode, const QString& newLabel) override;
//     bool updateUUID(Report& report, const QString& deviceNode) const override;

    CommandSupportType supportGetUsed() const override {
        return m_GetUsed;
    }
    CommandSupportType supportGetLabel() const override {
        return m_GetLabel;
    }
    CommandSupportType supportCreate() const override {
        return m_Create;
    }
    CommandSupportType supportCreateWithLabel() const override {
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
    CommandSupportType supportCheck() const override {
        return m_Check;
    }
    CommandSupportType supportCopy() const override {
        return m_Copy;
    }
    CommandSupportType supportBackup() const override {
        return m_Backup;
    }
    CommandSupportType supportSetLabel() const override {
        return m_SetLabel;
    }
    CommandSupportType supportUpdateUUID() const override {
        return m_UpdateUUID;
    }
    CommandSupportType supportGetUUID() const override {
        return m_GetUUID;
    }

    qint64 minCapacity() const override;
    qint64 maxCapacity() const override;
    int maxLabelLength() const override;
    SupportTool supportToolName() const override;
    bool supportToolFound() const override;

public:
    static CommandSupportType m_GetUsed;
    static CommandSupportType m_GetLabel;
    static CommandSupportType m_Create;
    static CommandSupportType m_Grow;
    static CommandSupportType m_Shrink;
    static CommandSupportType m_Move;
    static CommandSupportType m_Check;
    static CommandSupportType m_Copy;
    static CommandSupportType m_Backup;
    static CommandSupportType m_SetLabel;
    static CommandSupportType m_UpdateUUID;
    static CommandSupportType m_GetUUID;

private:
    static bool oldVersion;

};
}

#endif
