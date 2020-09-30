/*
    SPDX-FileCopyrightText: 2016 Chantara Tith <tith.chantara@gmail.com>
    SPDX-FileCopyrightText: 2016-2018 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2016 Teo Mrnjavac <teo@kde.org>
    SPDX-FileCopyrightText: 2018 Caio Jordão Carvalho <caiojcarvalho@gmail.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "ops/removevolumegroupoperation.h"
#include "jobs/removevolumegroupjob.h"

#include "core/lvmdevice.h"
#include "core/partition.h"
#include "core/partitiontable.h"
#include "core/volumemanagerdevice.h"

#include <QString>

#include <KLocalizedString>

/** Creates a new RemoveVolumeGroupOperation.
*/
RemoveVolumeGroupOperation::RemoveVolumeGroupOperation(VolumeManagerDevice& d) :
    Operation(),
    m_RemoveVolumeGroupJob(new RemoveVolumeGroupJob(d)),
    m_Device(d),
    m_PartitionTable(nullptr)
{
    addJob(removeVolumeGroupJob());
}

QString RemoveVolumeGroupOperation::description() const
{
    return xi18nc("@info/plain", "Remove a LVM volume group.");
}

void RemoveVolumeGroupOperation::preview()
{
    m_PartitionTable = device().partitionTable();

    if (device().type() == Device::Type::LVM_Device) {
        LvmDevice& lvm = static_cast<LvmDevice&>(device());

        LvmDevice::s_OrphanPVs << lvm.physicalVolumes();
    }

    device().setPartitionTable(new PartitionTable(PartitionTable::vmd, 0, device().totalLogical() - 1));
}

void RemoveVolumeGroupOperation::undo()
{
    if (device().type() == Device::Type::LVM_Device) {
        LvmDevice& lvm = static_cast<LvmDevice&>(device());

        const QVector<const Partition*> constOrphanList = LvmDevice::s_OrphanPVs;

        for (const Partition* p : constOrphanList)
            if (lvm.physicalVolumes().contains(p))
                LvmDevice::s_OrphanPVs.removeAll(p);
    }

    device().setPartitionTable(m_PartitionTable);
}

/** Check if Volume Group can be safely removed
 *
 *  @param dev VolumeManagerDevice with initialized partitions
 *  @return true if there are no LVM partitions.
 */
bool RemoveVolumeGroupOperation::isRemovable(const VolumeManagerDevice* dev)
{
    // TODO: allow removal when LVs are inactive.
    if (dev->type() == Device::Type::LVM_Device) {
        if (dev->partitionTable()->children().count() == 0) // This is necessary to prevent a crash during applying of operations
            return true;
        else if (dev->partitionTable()->children().count() > 1)
            return false;
        else
            if (dev->partitionTable()->children().first()->fileSystem().type() == FileSystem::Type::Unknown)
                return true;
    }

    return false;
}
