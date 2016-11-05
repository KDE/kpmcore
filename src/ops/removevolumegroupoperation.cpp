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

#include "core/partitiontable.h"
#include "core/volumemanagerdevice.h"

#include <QString>

#include <KLocalizedString>

/** Creates a new RemoveVolumeGroupOperation.
*/
RemoveVolumeGroupOperation::RemoveVolumeGroupOperation(VolumeManagerDevice& d) :
    Operation(),
    m_RemoveVolumeGroupJob(new RemoveVolumeGroupJob(d)),
    m_Device(d)
{
    addJob(removeVolumeGroupJob());
}

QString RemoveVolumeGroupOperation::description() const
{
    return xi18nc("@info/plain", "Remove a new LVM volume group.");
}

void RemoveVolumeGroupOperation::preview()
{
    m_PartitionTable = device().partitionTable();
    device().setPartitionTable(new PartitionTable(PartitionTable::vmd, 0, device().totalLogical() - 1));
}

void RemoveVolumeGroupOperation::undo()
{
    device().setPartitionTable(m_PartitionTable);
}
