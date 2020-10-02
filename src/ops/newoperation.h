/*
    SPDX-FileCopyrightText: 2008-2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2012-2016 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2015 Chris Campbell <c.j.campbell@ed.ac.uk>
    SPDX-FileCopyrightText: 2015 Teo Mrnjavac <teo@kde.org>
    SPDX-FileCopyrightText: 2020 Gaël PORTAY <gael.portay@collabora.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KPMCORE_NEWOPERATION_H
#define KPMCORE_NEWOPERATION_H

#include "fs/filesystem.h"
#include "ops/operation.h"
#include "util/libpartitionmanagerexport.h"

#include <QString>

class Device;
class OperationStack;

class CreatePartitionJob;
class SetPartitionLabelJob;
class SetPartitionUUIDJob;
class SetPartitionAttributesJob;
class CreateFileSystemJob;
class SetFileSystemLabelJob;
class SetPartFlagsJob;
class CheckFileSystemJob;

/** Create a Partition.

    Creates the given Partition on the given Device.

    @author Volker Lanz <vl@fidra.de>
*/
class LIBKPMCORE_EXPORT NewOperation : public Operation
{
    friend class OperationStack;

    Q_DISABLE_COPY(NewOperation)

public:
    NewOperation(Device& d, Partition* p);
    ~NewOperation();

public:
    QString iconName() const override {
        return QStringLiteral("document-new");
    }
    QString description() const override;
    void preview() override;
    void undo() override;

    bool targets(const Device& d) const override;
    bool targets(const Partition& p) const override;

    static bool canCreateNew(const Partition* p);
    static Partition* createNew(const Partition& cloneFrom, FileSystem::Type type);

protected:
    Partition& newPartition() {
        return *m_NewPartition;
    }
    const Partition& newPartition() const {
        return *m_NewPartition;
    }

    Device& targetDevice() {
        return m_TargetDevice;
    }
    const Device& targetDevice() const {
        return m_TargetDevice;
    }

    CreatePartitionJob* createPartitionJob() {
        return m_CreatePartitionJob;
    }
    SetPartitionLabelJob* setPartitionLabelJob() {
        return m_SetPartitionLabelJob;
    }
    SetPartitionUUIDJob* setPartitionUUIDJob() {
        return m_SetPartitionUUIDJob;
    }
    SetPartitionAttributesJob* setPartitionAttributesJob() {
        return m_SetPartitionAttributesJob;
    }
    CreateFileSystemJob* createFileSystemJob() {
        return m_CreateFileSystemJob;
    }
    SetPartFlagsJob* setPartFlagsJob() {
        return m_SetPartFlagsJob;
    }
    SetFileSystemLabelJob* setLabelJob() {
        return m_SetFileSystemLabelJob;
    }
    CheckFileSystemJob* checkJob() {
        return m_CheckFileSystemJob;
    }

private:
    Device& m_TargetDevice;
    Partition* m_NewPartition;
    CreatePartitionJob* m_CreatePartitionJob;
    SetPartitionLabelJob* m_SetPartitionLabelJob;
    SetPartitionUUIDJob* m_SetPartitionUUIDJob;
    SetPartitionAttributesJob* m_SetPartitionAttributesJob;
    CreateFileSystemJob* m_CreateFileSystemJob;
    SetPartFlagsJob* m_SetPartFlagsJob;
    SetFileSystemLabelJob* m_SetFileSystemLabelJob;
    CheckFileSystemJob* m_CheckFileSystemJob;
};

#endif
