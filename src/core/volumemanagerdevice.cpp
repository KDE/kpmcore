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

#include "core/volumemanagerdevice.h"

/** Constructs an abstract Volume Manager Device with an empty PartitionTable.
 *
*/
VolumeManagerDevice::VolumeManagerDevice(const QString& name,
                                         const QString& deviceNode,
                                         const qint64 logicalSize,
                                         const qint64 totalLogical,
                                         const QString& iconName,
                                         Device::Type type)
    : Device(name, deviceNode, logicalSize, totalLogical, iconName, type)
{
}

QString VolumeManagerDevice::prettyDeviceNodeList() const
{
    return deviceNodes().join(QStringLiteral(", "));
}
