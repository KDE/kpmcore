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

#include "ops/deactivatevolumegroupoperation.h"
#include "jobs/deactivatevolumegroupjob.h"
#include "jobs/deactivatelogicalvolumejob.h"

#include "core/volumemanagerdevice.h"
#include "core/partitiontable.h"
#include "core/partition.h"

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
    device().setPartitionTable(new PartitionTable(PartitionTable::vmd, 0, device().totalLogical() - 1));
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
    if (dev->type() == Device::Type::LVM_Device) {
        for (const auto &p : dev->partitionTable()->children()) {
            if (p->isMounted()) {
                return false;
            }
        }
        return true;
    }

    return false;
}
