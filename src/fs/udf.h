/*
    SPDX-FileCopyrightText: 2017 Pali Rohár <pali.rohar@gmail.com>
    SPDX-FileCopyrightText: 2017 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2020 Arnaud Ferraris <arnaud.ferraris@collabora.com>
    SPDX-FileCopyrightText: 2020 Gaël PORTAY <gael.portay@collabora.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KPMCORE_UDF_H
#define KPMCORE_UDF_H

#include "util/libpartitionmanagerexport.h"

#include "fs/filesystem.h"

#include <QtGlobal>

class Report;

class QString;

namespace FS
{
/** A udf file system.
    @author Pali Rohár <pali.rohar@gmail.com>
 */
class LIBKPMCORE_EXPORT udf : public FileSystem
{
public:
    udf(qint64 firstsector, qint64 lastsector, qint64 sectorsused, const QString& label, const QVariantMap& features = {});

public:
    void init() override;

    qint64 readUsedCapacity(const QString& deviceNode) const override;
    bool create(Report& report, const QString& deviceNode) override;
    bool createWithLabel(Report& report, const QString& deviceNode, const QString& label) override;
    bool writeLabel(Report& report, const QString& deviceNode, const QString& newLabel) override;
    bool updateUUID(Report& report, const QString& deviceNode) const override;

    CommandSupportType supportGetUsed() const override {
        return m_GetUsed;
    }
    CommandSupportType supportGetLabel() const override {
        return cmdSupportCore;
    }
    CommandSupportType supportCreate() const override {
        return m_Create;
    }
    CommandSupportType supportCreateWithLabel() const override {
        return m_Create;
    }
    CommandSupportType supportMove() const override {
        return cmdSupportCore;
    }
    CommandSupportType supportCopy() const override {
        return cmdSupportCore;
    }
    CommandSupportType supportBackup() const override {
        return cmdSupportCore;
    }
    CommandSupportType supportSetLabel() const override {
        return m_SetLabel;
    }
    CommandSupportType supportUpdateUUID() const override {
        return m_UpdateUUID;
    }
    CommandSupportType supportGetUUID() const override {
        return cmdSupportCore;
    }

    qint64 minCapacity() const override;
    qint64 maxCapacity() const override;
    int maxLabelLength() const override;
    QValidator* labelValidator(QObject *parent = nullptr) const override;
    SupportTool supportToolName() const override;
    bool supportToolFound() const override;

public:
    static CommandSupportType m_GetUsed;
    static CommandSupportType m_SetLabel;
    static CommandSupportType m_UpdateUUID;
    static CommandSupportType m_Create;

private:
    static bool oldMkudffsVersion;
};
}

#endif
