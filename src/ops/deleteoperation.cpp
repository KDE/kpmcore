/*
    SPDX-FileCopyrightText: 2008-2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2012-2018 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2015-2016 Teo Mrnjavac <teo@kde.org>
    SPDX-FileCopyrightText: 2018 Caio Jordão Carvalho <caiojcarvalho@gmail.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "ops/deleteoperation.h"

#include "core/partition.h"
#include "core/device.h"
#include "core/lvmdevice.h"
#include "core/partitiontable.h"
#include "core/raid/softwareraid.h"
#include "fs/luks.h"

#include "jobs/deletepartitionjob.h"
#include "jobs/deletefilesystemjob.h"
#include "jobs/shredfilesystemjob.h"

#include "util/capacity.h"

#include <QString>

#include <KLocalizedString>

/** Creates a new DeleteOperation
    @param d the Device to delete a Partition on
    @param p pointer to the Partition to delete. May not be nullptr
*/
DeleteOperation::DeleteOperation(Device& d, Partition* p, ShredAction shred) :
    Operation(),
    m_TargetDevice(d),
    m_DeletedPartition(p),
    m_ShredAction(shred),
    m_DeletePartitionJob(new DeletePartitionJob(targetDevice(), deletedPartition()))
{
    switch (shredAction()) {
    case ShredAction::NoShred:
        m_DeleteFileSystemJob = static_cast<Job*>(new DeleteFileSystemJob(targetDevice(), deletedPartition()));
        break;
    case ShredAction::ZeroShred:
        m_DeleteFileSystemJob = static_cast<Job*>(new ShredFileSystemJob(targetDevice(), deletedPartition(), false));
        break;
    case ShredAction::RandomShred:
        m_DeleteFileSystemJob = static_cast<Job*>(new ShredFileSystemJob(targetDevice(), deletedPartition(), true));
    }

    addJob(deleteFileSystemJob());
    if (d.partitionTable()->type() != PartitionTable::TableType::none)
        addJob(deletePartitionJob());
}

DeleteOperation::~DeleteOperation()
{
    if (status() != StatusPending && status() != StatusNone) // don't delete the partition if we're being merged or undone
        delete m_DeletedPartition;
}

bool DeleteOperation::targets(const Device& d) const
{
    return d == targetDevice();
}

bool DeleteOperation::targets(const Partition& p) const
{
    return p == deletedPartition();
}

void DeleteOperation::preview()
{
    removePreviewPartition(targetDevice(), deletedPartition());
    checkAdjustLogicalNumbers(deletedPartition(), false);
}

void DeleteOperation::undo()
{
    checkAdjustLogicalNumbers(deletedPartition(), true);
    insertPreviewPartition(targetDevice(), deletedPartition());
}

QString DeleteOperation::description() const
{
    if (shredAction() != ShredAction::NoShred)
        return xi18nc("@info:status", "Shred partition <filename>%1</filename> (%2, %3)", deletedPartition().deviceNode(), Capacity::formatByteSize(deletedPartition().capacity()), deletedPartition().fileSystem().name());
    else
        return xi18nc("@info:status", "Delete partition <filename>%1</filename> (%2, %3)", deletedPartition().deviceNode(), Capacity::formatByteSize(deletedPartition().capacity()), deletedPartition().fileSystem().name());
}

void DeleteOperation::checkAdjustLogicalNumbers(Partition& p, bool undo)
{
    // If the deleted partition is a logical one, we need to adjust the numbers of the
    // other logical partitions in the extended one, if there are any, because the OS
    // will do that, too: Logicals must be numbered without gaps, i.e., a numbering like
    // sda5, sda6, sda8 (after sda7 is deleted) will become sda5, sda6, sda7
    Partition* parentPartition = dynamic_cast<Partition*>(p.parent());
    if (parentPartition && parentPartition->roles().has(PartitionRole::Extended))
        parentPartition->adjustLogicalNumbers(undo ? -1 : p.number(), undo ? p.number() : -1);
}

/** Can a Partition be deleted?
    @param p the Partition in question, may be nullptr.
    @return true if @p p can be deleted.
*/
bool DeleteOperation::canDelete(const Partition* p)
{
    if (p == nullptr)
        return false;

    if (p->isMounted())
        return false;

    if (p->fileSystem().type() == FileSystem::Type::Lvm2_PV) {
        if (LvmDevice::s_DirtyPVs.contains(p))
            return false;
    }
    else if (p->fileSystem().type() == FileSystem::Type::LinuxRaidMember) {
        if (SoftwareRAID::isRaidMember(p->partitionPath()))
            return false;
    }
    else if (p->fileSystem().type() == FileSystem::Type::Luks || p->fileSystem().type() == FileSystem::Type::Luks2) {
        // See if innerFS is LVM
        FileSystem *fs = static_cast<const FS::luks *>(&p->fileSystem())->innerFS();

        if (fs) {
            if (fs->type() == FileSystem::Type::Lvm2_PV) {
                if (LvmDevice::s_DirtyPVs.contains(p))
                    return false;
            }
            else if (fs->type() == FileSystem::Type::LinuxRaidMember) {
                if (SoftwareRAID::isRaidMember(p->partitionPath()))
                    return false;
            }
        }
    }

    if (p->roles().has(PartitionRole::Unallocated))
        return false;

    if (p->roles().has(PartitionRole::Extended))
        return p->children().size() == 1 && p->children()[0]->roles().has(PartitionRole::Unallocated);

    if (p->roles().has(PartitionRole::Luks))
    {
        const FS::luks* luksFs = static_cast<const FS::luks*>(&p->fileSystem());
        if (!luksFs)
            return false;

        if (luksFs->isCryptOpen() || luksFs->isMounted())
            return false;
    }

    return true;
}
