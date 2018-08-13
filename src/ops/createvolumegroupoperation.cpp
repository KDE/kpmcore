/*************************************************************************
 *  Copyright (C) 2016 by Chantara Tith <tith.chantara@gmail.com>        *
 *  Copyright (C) 2016 by Andrius Štikonas <andrius@stikonas.eu>         *
 *  Copyright (C) 2018 by Caio Carvalho <caiojcarvalho@gmail.com>        *
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
#include "fs/lvm2_pv.h"

#include "jobs/createvolumegroupjob.h"

#include <QString>

#include <KLocalizedString>

/** Creates a new CreateVolumeGroupOperation.
 * @param vgName LVM Volume Group name
 * @param pvList List of LVM Physical Volumes used to create Volume Group
 * @param peSize LVM Physical Extent size in MiB
*/
CreateVolumeGroupOperation::CreateVolumeGroupOperation(const QString& vgName, const QVector<const Partition*>& pvList,
                                                       const Device::Type type, const qint32 peSize)
    : Operation()
    , m_CreateVolumeGroupJob(new CreateVolumeGroupJob(vgName, pvList, type, peSize))
    , m_PVList(pvList)
    , m_vgName(vgName)
{
    addJob(createVolumeGroupJob());
}

CreateVolumeGroupOperation::CreateVolumeGroupOperation(const QString &vgName, const QVector<const Partition *> &pvList,
                                                       const Device::Type type, const qint32 raidLevel,
                                                       const qint32 chunkSize)
    : Operation()
    , m_CreateVolumeGroupJob(new CreateVolumeGroupJob(vgName, pvList, type, raidLevel, chunkSize))
    , m_PVList(pvList)
    , m_vgName(vgName)
{
    addJob(createVolumeGroupJob());
}

QString CreateVolumeGroupOperation::description() const
{
    return xi18nc("@info/plain", "Create a new volume group named \'%1\'.", m_vgName);
}

bool CreateVolumeGroupOperation::targets(const Partition& partition) const
{
    for (const auto &p : m_PVList) {
        if (partition == *p)
            return true;
    }
    return false;
}

void CreateVolumeGroupOperation::preview()
{
    if (type() == Device::Type::LVM_Device)
        LvmDevice::s_DirtyPVs << PVList();
    else if (type() == Device::Type::SoftwareRAID_Device) {
        // TODO: Set it for RAID
    }
}

void CreateVolumeGroupOperation::undo()
{
    if (type() == Device::Type::LVM_Device) {
        for (const auto &pvPath : PVList())
            if (LvmDevice::s_DirtyPVs.contains(pvPath))
                LvmDevice::s_DirtyPVs.removeAll(pvPath);
    }
    else if (type() == Device::Type::SoftwareRAID_Device) {
        // TODO: Set it for RAID

    }
}

bool CreateVolumeGroupOperation::canCreate()
{
    return true;
}
