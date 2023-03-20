/*
    SPDX-FileCopyrightText: 2019 Shubham Jangra <aryan100jangid@gmail.com>
    SPDX-FileCopyrightText: 2020 Arnaud Ferraris <arnaud.ferraris@collabora.com>
    SPDX-FileCopyrightText: 2020 Gaël PORTAY <gael.portay@collabora.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/
 
#ifndef KPMCORE_MINIX_H
#define KPMCORE_MINIX_H

#include "fs/filesystem.h"

#include "util/libpartitionmanagerexport.h"

class Report;

class QString;

namespace FS
{
/** A minix(Mini Unix) file system.
    @author Shubham <aryan100jangid@gmail.com>
 */    
class LIBKPMCORE_EXPORT minix : public FileSystem
{
public:
    minix(qint64 firstsector, qint64 lastsector, qint64 sectorsused, const QString& label, const QVariantMap& features = {});

    void init() override;
    
    bool check(Report& report, const QString&deviceNode) const override;
    bool create(Report& report, const QString&deviceNode) override;

    QString posixPermissions() const override { return implPosixPermissions();  };
    void setPosixPermissions(const QString& permissions) override { implSetPosixPermissions(permissions); };

    CommandSupportType supportGetLabel() const override {
        return m_GetLabel;
    }
    
    CommandSupportType supportGetUsed() const override {
        return m_GetUsed;
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
    
    CommandSupportType supportCreate() const override {
        return m_Create;
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
    static CommandSupportType m_GetLabel;
    static CommandSupportType m_GetUsed;
    static CommandSupportType m_Shrink;
    static CommandSupportType m_Move;
    static CommandSupportType m_Create;
    static CommandSupportType m_Check;
    static CommandSupportType m_Copy;
    static CommandSupportType m_Backup;
};
}

#endif
