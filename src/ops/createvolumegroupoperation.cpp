/*
    SPDX-FileCopyrightText: 2016 Chantara Tith <tith.chantara@gmail.com>
    SPDX-FileCopyrightText: 2016-2017 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2018 Caio Jordão Carvalho <caiojcarvalho@gmail.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

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
