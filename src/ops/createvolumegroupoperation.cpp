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

#include "ops/createvolumegroupoperation.h"

#include "core/lvmdevice.h"

#include "jobs/createvolumegroupjob.h"

#include <QString>

#include <KLocalizedString>

/** Creates a new CreateVolumeGroupOperation.
    @param d the Device to create the new PartitionTable on
    @param t the type for the new PartitionTable
*/
CreateVolumeGroupOperation::CreateVolumeGroupOperation(const QString& vgname, const QList<Partition*> pvlist) :
    Operation(),
    m_CreateVolumeGroupJob(new CreateVolumeGroupJob(vgname, pvlist))
{
    addJob(createVolumeGroupJob());
}

bool CreateVolumeGroupOperation::canCreate()
{
    //TODO: return true if there is any free PV
    return true;
}

QString CreateVolumeGroupOperation::description() const
{
    return xi18nc("@info/plain", "Create a new LVM volume group.");
}

bool CreateVolumeGroupOperation::targets(const Device&) const
{
    return false;
}

bool CreateVolumeGroupOperation::targets(const Partition& part) const
{
    return true;
}

void CreateVolumeGroupOperation::preview()
{
}

void CreateVolumeGroupOperation::undo()
{
}
