/*
    SPDX-FileCopyrightText: 2008-2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2014-2016 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2015 Chris Campbell <c.j.campbell@ed.ac.uk>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KPMCORE_COPYOPERATION_H
#define KPMCORE_COPYOPERATION_H

#include "util/libpartitionmanagerexport.h"

#include "ops/operation.h"

#include <QString>

class Partition;
class OperationStack;
class Device;
class Report;

class CreatePartitionJob;
class CheckFileSystemJob;
class CopyFileSystemJob;
class ResizeFileSystemJob;

/** Copy a Partition.

    Copies a Partition from a given source Device to a Partition on a given target Device and handles overwriting
    the target Partition in case that is required.

    @author Volker Lanz <vl@fidra.de>
*/
class LIBKPMCORE_EXPORT CopyOperation : public Operation
{
    friend class OperationStack;

    Q_DISABLE_COPY(CopyOperation)

public:
    CopyOperation(Device& targetdevice, Partition* copiedpartition, Device& sourcedevice, Partition* sourcepartition);
    ~CopyOperation();

public:
    QString iconName() const override {
        return QStringLiteral("edit-copy");
    }
    QString description() const override {
        return m_Description;
    }

    bool execute(Report& parent) override;
    void preview() override;
    void undo() override;

    bool targets(const Device& d) const override;
    bool targets(const Partition& p) const override;

    static bool canCopy(const Partition* p);
    static bool canPaste(const Partition* p, const Partition* source);

    static Partition* createCopy(const Partition& target, const Partition& source);

protected:
    Partition& copiedPartition() {
        return *m_CopiedPartition;
    }
    const Partition& copiedPartition() const {
        return *m_CopiedPartition;
    }

    Device& targetDevice() {
        return m_TargetDevice;
    }
    const Device& targetDevice() const {
        return m_TargetDevice;
    }

    Device& sourceDevice() {
        return m_SourceDevice;
    }
    const Device& sourceDevice() const {
        return m_SourceDevice;
    }

    Partition& sourcePartition() {
        return *m_SourcePartition;
    }
    const Partition& sourcePartition() const {
        return *m_SourcePartition;
    }

    Partition* overwrittenPartition() {
        return m_OverwrittenPartition;
    }
    const Partition* overwrittenPartition() const {
        return m_OverwrittenPartition;
    }

    void setOverwrittenPartition(Partition* p);
    void setSourcePartition(Partition* p) {
        m_SourcePartition = p;
    }

    void cleanupOverwrittenPartition();
    bool mustDeleteOverwritten() const {
        return m_MustDeleteOverwritten;
    }

    CheckFileSystemJob* checkSourceJob() {
        return m_CheckSourceJob;
    }
    CreatePartitionJob* createPartitionJob() {
        return m_CreatePartitionJob;
    }
    CopyFileSystemJob* copyFSJob() {
        return m_CopyFSJob;
    }
    CheckFileSystemJob* checkTargetJob() {
        return m_CheckTargetJob;
    }
    ResizeFileSystemJob* maximizeJob() {
        return m_MaximizeJob;
    }

    QString updateDescription() const;

private:
    Device& m_TargetDevice;
    Partition* m_CopiedPartition;
    Device& m_SourceDevice;
    Partition* m_SourcePartition;
    Partition* m_OverwrittenPartition;
    bool m_MustDeleteOverwritten;

    CheckFileSystemJob* m_CheckSourceJob;
    CreatePartitionJob* m_CreatePartitionJob;
    CopyFileSystemJob* m_CopyFSJob;
    CheckFileSystemJob* m_CheckTargetJob;
    ResizeFileSystemJob* m_MaximizeJob;

    QString m_Description;
};

#endif
