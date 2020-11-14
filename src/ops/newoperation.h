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

struct NewOperationPrivate;
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
    Partition& newPartition();
    const Partition& newPartition() const;

    Device& targetDevice();
    const Device& targetDevice() const;

    CreatePartitionJob* createPartitionJob();
    SetPartitionLabelJob* setPartitionLabelJob();
    SetPartitionUUIDJob* setPartitionUUIDJob();
    SetPartitionAttributesJob* setPartitionAttributesJob();
    CreateFileSystemJob* createFileSystemJob();
    SetPartFlagsJob* setPartFlagsJob();
    SetFileSystemLabelJob* setLabelJob();
    CheckFileSystemJob* checkJob();

private:
    std::unique_ptr<NewOperationPrivate> d_ptr;
};

#endif
