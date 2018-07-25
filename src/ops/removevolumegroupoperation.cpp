/*************************************************************************
 *  Copyright (C) 2016 by Chantara Tith <tith.chantara@gmail.com>        *
 *                                                                       *
 *  This program is free software; you can redistribute it and/or        *
 *  modify it under the terms of the GNU General Public License as       *
 *  published by the Free Software Foundation; either version 3 of       *
 *  the License, or (at your option) any later version.                  *
 *                                                                       *
 *  This program is distributed in the hope that it will be useful,      *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of       *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        *
 *  GNU General Public License for more details.                         *
 *                                                                       *
 *  You should have received a copy of the GNU General Public License    *
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.*
 *************************************************************************/

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
