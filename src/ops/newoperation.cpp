/*
    SPDX-FileCopyrightText: 2008-2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2012-2019 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2015 Teo Mrnjavac <teo@kde.org>
    SPDX-FileCopyrightText: 2020 Gaël PORTAY <gael.portay@collabora.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "ops/newoperation.h"

#include "core/partition.h"
#include "core/device.h"
#include "core/partitionnode.h"

#include "jobs/createpartitionjob.h"
#include "jobs/createfilesystemjob.h"
#include "jobs/setpartitionlabeljob.h"
#include "jobs/setpartitionuuidjob.h"
#include "jobs/setpartitionattributesjob.h"
#include "jobs/setfilesystemlabeljob.h"
#include "jobs/setpartflagsjob.h"
#include "jobs/checkfilesystemjob.h"
#include "jobs/changepermissionsjob.h"

#include "fs/filesystem.h"
#include "fs/filesystemfactory.h"

#include "util/capacity.h"

#include <QString>

#include <KLocalizedString>

struct NewOperationPrivate
{
    NewOperationPrivate(Device& d, Partition* p) :
        m_TargetDevice(d),
        m_NewPartition(p),
        m_CreatePartitionJob(new CreatePartitionJob(d, *p)),
        m_SetPartitionLabelJob(nullptr),
        m_SetPartitionUUIDJob(nullptr),
        m_SetPartitionAttributesJob(nullptr),
        m_CreateFileSystemJob(nullptr),
        m_SetPartFlagsJob(nullptr),
        m_SetFileSystemLabelJob(nullptr),
        m_CheckFileSystemJob(nullptr)
    {
    }

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

/** Creates a new NewOperation.
    @param d the Device to create a new Partition on
    @param p pointer to the new Partition to create. May not be nullptr.
*/
NewOperation::NewOperation(Device& d, Partition* p) :
    Operation(),
    d_ptr(std::make_unique<NewOperationPrivate>(d, p))
{
    addJob(createPartitionJob());

    if (!p->label().isEmpty()) {
        d_ptr->m_SetPartitionLabelJob = new SetPartitionLabelJob(targetDevice(), newPartition(), p->label());
        addJob(setPartitionLabelJob());
    }

    if (!p->uuid().isEmpty()) {
        d_ptr->m_SetPartitionUUIDJob = new SetPartitionUUIDJob(targetDevice(), newPartition(), p->uuid());
        addJob(setPartitionUUIDJob());
    }

    if (p->attributes()) {
        d_ptr->m_SetPartitionAttributesJob = new SetPartitionAttributesJob(targetDevice(), newPartition(), p->attributes());
        addJob(setPartitionAttributesJob());
    }

    const FileSystem& fs = newPartition().fileSystem();

    if (fs.type() != FileSystem::Type::Extended) {
        // It would seem tempting to skip the CreateFileSystemJob or the
        // SetFileSystemLabelJob if either has nothing to do (unformatted FS or
        // empty label). However, the user might later on decide to change FS or
        // label. The operation stack will merge these operations with this one here
        // and if the jobs don't exist things will break.

        d_ptr->m_CreateFileSystemJob = new CreateFileSystemJob(targetDevice(), newPartition(), fs.label());
        addJob(createFileSystemJob());

        if (fs.type() == FileSystem::Type::Lvm2_PV) {
            d_ptr->m_SetPartFlagsJob = new SetPartFlagsJob(targetDevice(), newPartition(), PartitionTable::Flag::Lvm);
            addJob(setPartFlagsJob());
        }

        d_ptr->m_SetFileSystemLabelJob = new SetFileSystemLabelJob(newPartition(), fs.label());
        addJob(setLabelJob());

        d_ptr->m_CheckFileSystemJob = new CheckFileSystemJob(newPartition());
        addJob(checkJob());

        // if the user never configured a new permission, nothing will run, if he did,
        // then we change the permissions on the newly created partition.
        addJob(new ChangePermissionJob(newPartition()));
    }
}

NewOperation::~NewOperation()
{
    if (status() == StatusPending)
        delete d_ptr->m_NewPartition;
}

Partition& NewOperation::newPartition()
{
    return *d_ptr->m_NewPartition;
}

const Partition& NewOperation::newPartition() const
{
    return *d_ptr->m_NewPartition;
}

Device& NewOperation::targetDevice()
{
    return d_ptr->m_TargetDevice;
}

const Device& NewOperation::targetDevice() const
{
    return d_ptr->m_TargetDevice;
}

CreatePartitionJob* NewOperation::createPartitionJob()
{
    return d_ptr->m_CreatePartitionJob;
}

SetPartitionLabelJob* NewOperation::setPartitionLabelJob()
{
    return d_ptr->m_SetPartitionLabelJob;
}

SetPartitionUUIDJob* NewOperation::setPartitionUUIDJob()
{
    return d_ptr->m_SetPartitionUUIDJob;
}

SetPartitionAttributesJob* NewOperation::setPartitionAttributesJob()
{
    return d_ptr->m_SetPartitionAttributesJob;
}

CreateFileSystemJob* NewOperation::createFileSystemJob()
{
    return d_ptr->m_CreateFileSystemJob;
}

SetPartFlagsJob* NewOperation::setPartFlagsJob()
{
    return d_ptr->m_SetPartFlagsJob;
}

SetFileSystemLabelJob* NewOperation::setLabelJob()
{
    return d_ptr->m_SetFileSystemLabelJob;
}

CheckFileSystemJob* NewOperation::checkJob()
{
    return d_ptr->m_CheckFileSystemJob;
}

bool NewOperation::targets(const Device& d) const
{
    return d == targetDevice();
}

bool NewOperation::targets(const Partition& p) const
{
    return p == newPartition();
}

void NewOperation::preview()
{
    insertPreviewPartition(targetDevice(), newPartition());
}

void NewOperation::undo()
{
    removePreviewPartition(targetDevice(), newPartition());
}

QString NewOperation::description() const
{
    return xi18nc("@info:status", "Create a new partition (%1, %2) on <filename>%3</filename>", Capacity::formatByteSize(newPartition().capacity()), newPartition().fileSystem().name(), targetDevice().deviceNode());
}

/** Can a Partition be created somewhere?
    @param p the Partition where a new Partition is to be created, may be nullptr
    @return true if a new Partition can be created in @p p
 */
bool NewOperation::canCreateNew(const Partition* p)
{
    return p != nullptr && p->roles().has(PartitionRole::Unallocated);
}

Partition* NewOperation::createNew(const Partition& cloneFrom,
                                   FileSystem::Type type)
{
    Partition* p = new Partition(cloneFrom);

    p->deleteFileSystem();
    p->setFileSystem(FileSystemFactory::create(type,
                     p->firstSector(),
                     p->lastSector(),
                     p->sectorSize()));
    p->setState(Partition::State::New);
    p->setPartitionPath(QString());
    p->setAttributes(0);

    return p;
}
