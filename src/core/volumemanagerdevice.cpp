/*************************************************************************
 *  Copyright (C) 2016 by Chantara Tith <tith.chantara@gmail.com>        *
 *  Copyright (C) 2018 by Andrius Štikonas <andrius@stikonas.eu>         *
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

#include "core/device_p.h"
#include "core/volumemanagerdevice.h"
#include "core/volumemanagerdevice_p.h"

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

QString VolumeManagerDevice::prettyDeviceNodeList() const
{
    return deviceNodes().join(QStringLiteral(", "));
}

void VolumeManagerDevice::setTotalLogical(qint64 n)
{
    Q_ASSERT(n > 0);
    d->m_TotalLogical = n;
}
