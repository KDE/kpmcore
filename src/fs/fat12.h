/*
    SPDX-FileCopyrightText: 2008-2011 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2013-2018 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2015 Teo Mrnjavac <teo@kde.org>
    SPDX-FileCopyrightText: 2020 Arnaud Ferraris <arnaud.ferraris@collabora.com>
    SPDX-FileCopyrightText: 2020 Gaël PORTAY <gael.portay@collabora.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KPMCORE_FAT12_H
#define KPMCORE_FAT12_H

#include "util/libpartitionmanagerexport.h"

#include "fs/filesystem.h"

#include <QtGlobal>

class Report;

class QString;

namespace FS
{
/** A fat12 file system.
    @author Andrius Štikonas <vl@fidra.de>
 */
class LIBKPMCORE_EXPORT fat12 : public FileSystem
{
public:
    fat12(qint64 firstsector, qint64 lastsector, qint64 sectorsused, const QString& label, const QVariantMap& features = {}, FileSystem::Type t = FileSystem::Type::Fat12);

public:
    void init() override;

    qint64 readUsedCapacity(const QString& deviceNode) const override;
    bool check(Report& report, const QString& deviceNode) const override;
    bool create(Report& report, const QString& deviceNode) override;
    bool updateUUID(Report& report, const QString& deviceNode) const override;
    bool writeLabel(Report& report, const QString& deviceNode, const QString& newLabel) override;

    CommandSupportType supportGetUsed() const override {
        return m_GetUsed;
    }
    CommandSupportType supportGetLabel() const override {
        return m_GetLabel;
    }
    CommandSupportType supportSetLabel() const override {
        return m_SetLabel;
    }
    CommandSupportType supportCreate() const override {
        return m_Create;
    }
    CommandSupportType supportGrow() const override {
        return cmdSupportNone;
    }
    CommandSupportType supportShrink() const override {
        return cmdSupportNone;
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
    CommandSupportType supportUpdateUUID() const override {
        return m_UpdateUUID;
    }
    CommandSupportType supportGetUUID() const override {
        return m_GetUUID;
    }

    qint64 minCapacity() const override;
    qint64 maxCapacity() const override;
    int maxLabelLength() const override;
    QValidator* labelValidator(QObject *parent) const override;
    SupportTool supportToolName() const override;
    bool supportToolFound() const override;

protected:
    bool createWithFatSize(Report &report, const QString& deviceNode, int fatSize);

public:
    static CommandSupportType m_GetUsed;
    static CommandSupportType m_GetLabel;
    static CommandSupportType m_SetLabel;
    static CommandSupportType m_Create;
    static CommandSupportType m_Grow;
    static CommandSupportType m_Shrink;
    static CommandSupportType m_Move;
    static CommandSupportType m_Check;
    static CommandSupportType m_Copy;
    static CommandSupportType m_Backup;
    static CommandSupportType m_UpdateUUID;
    static CommandSupportType m_GetUUID;
};
}

#endif
