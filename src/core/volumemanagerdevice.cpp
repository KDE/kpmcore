/*
    SPDX-FileCopyrightText: 2016 Chantara Tith <tith.chantara@gmail.com>
    SPDX-FileCopyrightText: 2016-2018 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2019 Yuri Chornoivan <yurchor@ukr.net>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "core/volumemanagerdevice.h"
#include "core/volumemanagerdevice_p.h"
#include "core/device_p.h"
#include "core/lvmdevice.h"
#include "core/raid/softwareraid.h"

/** Constructs an abstract Volume Manager Device with an empty PartitionTable.
 *
 * @param name the Device's name
 * @param deviceNode the Device's node
 * @param logicalExtentSize the logical extent size that device uses
*/
VolumeManagerDevice::VolumeManagerDevice(std::shared_ptr<VolumeManagerDevicePrivate> d,
                                         const QString& name,
                                         const QString& deviceNode,
                                         const qint64 logicalExtentSize,
                                         const qint64 totalLogical,
                                         const QString& iconName,
                                         Device::Type type)
    : Device(std::static_pointer_cast<DevicePrivate>(d), name, deviceNode, logicalExtentSize, totalLogical, iconName, type)
{
}

void VolumeManagerDevice::scanDevices(QList<Device*>& devices)
{
    SoftwareRAID::scanSoftwareRAID(devices);
    LvmDevice::scanSystemLVM(devices); // LVM scanner needs all other devices, so should be last
}

QString VolumeManagerDevice::prettyDeviceNodeList() const
{
    return deviceNodes().join(QStringLiteral(", "));
}

void VolumeManagerDevice::setTotalLogical(qint64 n)
{
    Q_ASSERT(n > 0);
    d->m_TotalLogical = n;
}
