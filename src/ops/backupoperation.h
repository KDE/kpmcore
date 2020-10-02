/*
    SPDX-FileCopyrightText: 2008-2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2014-2018 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2015 Chris Campbell <c.j.campbell@ed.ac.uk>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KPMCORE_BACKUPOPERATION_H
#define KPMCORE_BACKUPOPERATION_H

#include "util/libpartitionmanagerexport.h"

#include "ops/operation.h"

#include <QString>

class Partition;
class Device;
class BackupFileSystemJob;

/** Back up a FileSystem.
    @author Volker Lanz <vl@fidra.de>
*/
class LIBKPMCORE_EXPORT BackupOperation : public Operation
{
    Q_DISABLE_COPY(BackupOperation)

public:
    BackupOperation(Device& targetDevice, Partition& backupPartition, const QString& filename);

public:
    QString iconName() const override {
        return QStringLiteral("document-export");
    }
    QString description() const override;
    void preview() override {}
    void undo() override {}

    bool targets(const Device&) const override {
        return false;
    }
    bool targets(const Partition&) const override{
        return false;
    }

    static bool canBackup(const Partition* p);

protected:
    Device& targetDevice() {
        return m_TargetDevice;
    }

    const Device& targetDevice() const {
        return m_TargetDevice;
    }

    Partition& backupPartition() {
        return m_BackupPartition;
    }
    const Partition& backupPartition() const {
        return m_BackupPartition;
    }

    const QString& fileName() const {
        return m_FileName;
    }

    BackupFileSystemJob* backupJob() {
        return m_BackupJob;
    }

private:
    Device& m_TargetDevice;
    Partition& m_BackupPartition;
    const QString m_FileName;
    BackupFileSystemJob* m_BackupJob;
};

#endif
