/*
    SPDX-FileCopyrightText: 2008-2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2014-2018 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2016 Chantara Tith <tith.chantara@gmail.com>
    SPDX-FileCopyrightText: 2018 Caio Jordão Carvalho <caiojcarvalho@gmail.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "ops/createpartitiontableoperation.h"

#include "core/device.h"
#include "core/partitiontable.h"
#include "core/partition.h"
#include "core/raid/softwareraid.h"

#include "jobs/createpartitiontablejob.h"

#include <QString>

#include <KLocalizedString>

/** Creates a new CreatePartitionTableOperation.
    @param d the Device to create the new PartitionTable on
    @param t the type for the new PartitionTable
*/
CreatePartitionTableOperation::CreatePartitionTableOperation(Device& d, PartitionTable::TableType t) :
    Operation(),
    m_TargetDevice(d),
    m_OldPartitionTable(targetDevice().partitionTable()),
    m_PartitionTable(new PartitionTable(t, PartitionTable::defaultFirstUsable(d, t), PartitionTable::defaultLastUsable(d, t))),
    m_CreatePartitionTableJob(new CreatePartitionTableJob(targetDevice()))
{
    addJob(createPartitionTableJob());
}

/** Creates a new CreatePartitionTableOperation.
    @param d the Device to create the new PartitionTable on
    @param ptable pointer to the new partition table object. the operation takes ownership.
*/
CreatePartitionTableOperation::CreatePartitionTableOperation(Device& d, PartitionTable* ptable) :
    Operation(),
    m_TargetDevice(d),
    m_OldPartitionTable(targetDevice().partitionTable()),
    m_PartitionTable(ptable),
    m_CreatePartitionTableJob(new CreatePartitionTableJob(targetDevice()))
{
    addJob(createPartitionTableJob());
}

CreatePartitionTableOperation::~CreatePartitionTableOperation()
{
    if (status() == StatusPending)
        delete m_PartitionTable;
}

bool CreatePartitionTableOperation::targets(const Device& d) const
{
    return d == targetDevice();
}

void CreatePartitionTableOperation::preview()
{
    targetDevice().setPartitionTable(partitionTable());
    targetDevice().partitionTable()->updateUnallocated(targetDevice());
}

void CreatePartitionTableOperation::undo()
{
    targetDevice().setPartitionTable(oldPartitionTable());

    if (targetDevice().partitionTable())
        targetDevice().partitionTable()->updateUnallocated(targetDevice());
}

bool CreatePartitionTableOperation::execute(Report& parent)
{
    targetDevice().setPartitionTable(partitionTable());
    return Operation::execute(parent);
}

/** Can a new partition table be created on a device?
    @param device pointer to the device, can be nullptr
    @return true if a new partition table can be created on @p device
*/
bool CreatePartitionTableOperation::canCreate(const Device* device)
{
    if (device == nullptr)
        return false;

    if (device->type() == Device::Type::SoftwareRAID_Device) {
        const SoftwareRAID* raid = static_cast<const SoftwareRAID *>(device);

        if (raid->status() == SoftwareRAID::Status::Inactive)
            return false;
    }

    return (device->partitionTable() == nullptr || !device->partitionTable()->isChildMounted())
            && (device->type() != Device::Type::LVM_Device);
}

QString CreatePartitionTableOperation::description() const
{
    return xi18nc("@info:status", "Create a new partition table (type: %1) on <filename>%2</filename>", partitionTable()->typeName(), targetDevice().deviceNode());
}
