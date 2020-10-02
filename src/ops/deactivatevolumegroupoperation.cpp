/*
    SPDX-FileCopyrightText: 2016 Chantara Tith <tith.chantara@gmail.com>
    SPDX-FileCopyrightText: 2016-2018 Andrius Štikonas <andrius@stikonas.eu>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "ops/deactivatevolumegroupoperation.h"
#include "jobs/deactivatevolumegroupjob.h"
#include "jobs/deactivatelogicalvolumejob.h"

#include "core/volumemanagerdevice.h"
#include "core/partitiontable.h"
#include "core/partition.h"
#include "core/raid/softwareraid.h"

#include <QString>

#include <KLocalizedString>

/** Creates a new RemoveVolumeGroupOperation.
*/
DeactivateVolumeGroupOperation::DeactivateVolumeGroupOperation(VolumeManagerDevice& d) :
    Operation(),
    m_DeactivateVolumeGroupJob(new DeactivateVolumeGroupJob(d)),
    m_DeactivateLogicalVolumeJob(new DeactivateLogicalVolumeJob(d)),
    m_Device(d),
    m_PartitionTable(d.partitionTable())
{
    addJob(deactivateLogicalVolumeJob());
    addJob(deactivateVolumeGroupJob());
}

QString DeactivateVolumeGroupOperation::description() const
{
    return xi18nc("@info/plain", "Deactivate volume group.");
}

void DeactivateVolumeGroupOperation::preview()
{
    m_PartitionTable = device().partitionTable();
    if (device().type() == Device::Type::LVM_Device)
        device().setPartitionTable(new PartitionTable(PartitionTable::vmd, 0, device().totalLogical() - 1));
    else if (device().type() == Device::Type::SoftwareRAID_Device && device().partitionTable() != nullptr)
        device().setPartitionTable(new PartitionTable(device().partitionTable()->type(), 0, device().totalLogical() - 1));
}

void DeactivateVolumeGroupOperation::undo()
{
    device().setPartitionTable(m_PartitionTable);
}

/** loop through given device's partitions to check if any is mounted.
 *
 *  @param dev VolumeManagerDevice with initialized partitions
 *  @return false if any of the device's partition is mounted.
 */
bool DeactivateVolumeGroupOperation::isDeactivatable(const VolumeManagerDevice* dev)
{
    if (dev->type() != Device::Type::SoftwareRAID_Device &&
            dev->type() != Device::Type::LVM_Device)
        return false;

    if (dev->partitionTable()) {
        for (const auto &p : dev->partitionTable()->children())
            if (p->isMounted())
                return false;
    }
    
    if (dev->type() == Device::Type::SoftwareRAID_Device) {
        const SoftwareRAID* raid = static_cast<const SoftwareRAID*>(dev);
        return raid->status() == SoftwareRAID::Status::Active;
    }

    return true;
}
