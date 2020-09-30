/*
    SPDX-FileCopyrightText: 2008-2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2014-2016 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2015 Chris Campbell <c.j.campbell@ed.ac.uk>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KPMCORE_RESTOREOPERATION_H
#define KPMCORE_RESTOREOPERATION_H

#include "util/libpartitionmanagerexport.h"

#include "ops/operation.h"

#include <QString>

class Partition;
class Device;
class OperationStack;
class Report;
class PartitionNode;

class CreatePartitionJob;
class RestoreFileSystemJob;
class CheckFileSystemJob;
class ResizeFileSystemJob;

/** Restore a Partition.

    Restores the FileSystem from a file to the given Partition on the given Device, handling overwriting
    a previous Partition in case that is necessary.

    @author Volker Lanz <vl@fidra.de>
*/
class LIBKPMCORE_EXPORT RestoreOperation : public Operation
{
    friend class OperationStack;

    Q_DISABLE_COPY(RestoreOperation)

public:
    RestoreOperation(Device& d, Partition* p, const QString& filename);
    ~RestoreOperation();

public:
    QString iconName() const override {
        return QStringLiteral("document-import");
    }
    QString description() const override;
    bool execute(Report& parent) override;
    void undo() override;

    void preview() override;

    bool targets(const Device& d) const override;
    bool targets(const Partition& p) const override;

    static bool canRestore(const Partition* p);
    static Partition* createRestorePartition(const Device& device, PartitionNode& parent, qint64 start, const QString& fileName);

protected:
    Device& targetDevice() {
        return m_TargetDevice;
    }
    const Device& targetDevice() const {
        return m_TargetDevice;
    }

    Partition& restorePartition() {
        return *m_RestorePartition;
    }
    const Partition& restorePartition() const {
        return *m_RestorePartition;
    }

    const QString& fileName() const {
        return m_FileName;
    }

    Partition* overwrittenPartition() {
        return m_OverwrittenPartition;
    }
    const Partition* overwrittenPartition() const {
        return m_OverwrittenPartition;
    }
    void setOverwrittenPartition(Partition* p);

    void cleanupOverwrittenPartition();
    bool mustDeleteOverwritten() const {
        return m_MustDeleteOverwritten;
    }

    qint64 imageLength() const {
        return m_ImageLength;
    }

    CreatePartitionJob* createPartitionJob() {
        return m_CreatePartitionJob;
    }
    RestoreFileSystemJob* restoreJob() {
        return m_RestoreJob;
    }
    CheckFileSystemJob* checkTargetJob() {
        return m_CheckTargetJob;
    }
    ResizeFileSystemJob* maximizeJob() {
        return m_MaximizeJob;
    }

private:
    Device& m_TargetDevice;
    Partition* m_RestorePartition;
    const QString m_FileName;
    Partition* m_OverwrittenPartition;
    bool m_MustDeleteOverwritten;
    qint64 m_ImageLength;
    CreatePartitionJob* m_CreatePartitionJob;
    RestoreFileSystemJob* m_RestoreJob;
    CheckFileSystemJob* m_CheckTargetJob;
    ResizeFileSystemJob* m_MaximizeJob;
};

#endif
