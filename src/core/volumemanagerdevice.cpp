/*************************************************************************
 *  Copyright (C) 2008 by Volker Lanz <vl@fidra.de>                      *
 *  Copyright (C) 2016 by Andrius Štikonas <andrius@stikonas.eu>         *
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

#include "core/partitiontable.h"
#include "core/smartstatus.h"

#include "util/capacity.h"

/** Constructs a Device with an empty PartitionTable.
*/
VolumeManagerDevice::VolumeManagerDevice(const QString& name,
                                         const QString& devicenode,
                                         const qint32 logicalSize,
                                         const qint64 totalLogical,
                                         const QString& iconname,
                                         Device::Type type)
    : Device(name, devicenode, logicalSize, totalLogical, iconname, type)
{
}
