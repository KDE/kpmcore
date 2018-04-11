/*************************************************************************
 *  Copyright (C) 2008 by Volker Lanz <vl@fidra.de>                      *
 *  Copyright (C) 2016-2018 by Andrius Štikonas <andrius@stikonas.eu>    *
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

#include "core/device.h"
#include "core/device_p.h"
#include "core/partitiontable.h"
#include "core/smartstatus.h"

#include "util/capacity.h"

#include <KLocalizedString>

/** Constructs a Device with an empty PartitionTable.
    @param name the Device's name, usually some string defined by the manufacturer
    @param deviceNode the Device's node, for example "/dev/sda"
*/
Device::Device(std::shared_ptr<DevicePrivate> d_ptr,
               const QString& name,
               const QString& deviceNode,
               const qint64 logicalSectorSize,
               const qint64 totalLogicalSectors,
               const QString& iconName,
               Device::Type type)
    : QObject()
    , d(d_ptr)
{
    d->m_Name = name.length() > 0 ? name : i18n("Unknown Device");
    d->m_DeviceNode = deviceNode;
    d->m_LogicalSectorSize = logicalSectorSize;
    d->m_TotalLogical = totalLogicalSectors;
    d->m_PartitionTable = nullptr;
    d->m_IconName = iconName.isEmpty() ? QStringLiteral("drive-harddisk") : iconName;
    d->m_SmartStatus = type == Device::Type::Disk_Device ? std::make_shared<SmartStatus>(deviceNode) : nullptr;
    d->m_Type = type;
}

/** Copy constructor for Device.
 * @param other the other Device.
 */
Device::Device(const Device& other)
    : QObject()
    , d(std::make_shared<DevicePrivate>())
{
    d->m_Name = other.d->m_Name;
    d->m_DeviceNode = other.d->m_DeviceNode;
    d->m_LogicalSectorSize = other.d->m_LogicalSectorSize;
    d->m_TotalLogical = other.d->m_TotalLogical;
    d->m_PartitionTable = nullptr;
    d->m_IconName = other.d->m_IconName;
    d->m_SmartStatus = nullptr;
    d->m_Type = other.d->m_Type;
    d->m_SmartStatus = other.d->m_SmartStatus;

    if (other.d->m_PartitionTable)
        d->m_PartitionTable = new PartitionTable(*other.d->m_PartitionTable);
}

/** Destructs a Device. */
Device::~Device()
{
    delete d->m_PartitionTable;
}

bool Device::operator==(const Device& other) const
{
    return d->m_DeviceNode == other.d->m_DeviceNode;
}

bool Device::operator!=(const Device& other) const
{
    return !(other == *this);
}

QString Device::prettyName() const
{
    return xi18nc("@item:inlistbox Device name – Capacity (device node)", "%1 – %2 (%3)", name(), Capacity::formatByteSize(capacity()), deviceNode());
}

QString& Device::name()
{
    return d->m_Name;
}

const QString& Device::name() const
{
    return d->m_Name;
}

const QString& Device::deviceNode() const
{
    return d->m_DeviceNode;
}

qint64 Device::logicalSize() const
{
    return d->m_LogicalSectorSize;
}

qint64 Device::totalLogical() const
{
    return d->m_TotalLogical;
}

PartitionTable* Device::partitionTable()
{
    return d->m_PartitionTable;
}

const PartitionTable* Device::partitionTable() const
{
    return d->m_PartitionTable;
}

void Device::setPartitionTable(PartitionTable* ptable)
{
    d->m_PartitionTable = ptable;
}

const QString& Device::iconName() const
{
    return d->m_IconName;
}

void Device::setIconName(const QString& name)
{
    d->m_IconName = name;
}

SmartStatus& Device::smartStatus()
{
    return *(d->m_SmartStatus);
}

const SmartStatus& Device::smartStatus() const
{
    return *(d->m_SmartStatus);
}

Device::Type Device::type() const
{
    return d->m_Type;
}
