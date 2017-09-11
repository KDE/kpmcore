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

#include "core/device.h"
#include "core/partitiontable.h"
#include "core/smartstatus.h"

#include "util/capacity.h"

#include <KLocalizedString>

/** Constructs a Device with an empty PartitionTable.
    @param name the Device's name, usually some string defined by the manufacturer
    @param deviceNode the Device's node, for example "/dev/sda"
*/
Device::Device(const QString& name,
               const QString& deviceNode,
               const qint64 logicalSize,
               const qint64 totalLogical,
               const QString& iconName,
               Device::Type type)
    : QObject()
    , m_Name(name.length() > 0 ? name : i18n("Unknown Device"))
    , m_DeviceNode(deviceNode)
    , m_LogicalSize(logicalSize)
    , m_TotalLogical(totalLogical)
    , m_PartitionTable(nullptr)
    , m_IconName(iconName.isEmpty() ? QStringLiteral("drive-harddisk") : iconName)
    , m_SmartStatus(type == Device::Disk_Device ? new SmartStatus(deviceNode) : nullptr)
    , m_Type(type)
{
}

/** Copy constructor for Device.
 * @param other the other Device.
 */
Device::Device(const Device& other)
    : QObject()
    , m_Name(other.m_Name)
    , m_DeviceNode(other.m_DeviceNode)
    , m_LogicalSize(other.m_LogicalSize)
    , m_TotalLogical(other.m_TotalLogical)
    , m_PartitionTable(nullptr)
    , m_IconName(other.m_IconName)
    , m_SmartStatus(nullptr)
    , m_Type(other.m_Type)
{
    if (other.m_PartitionTable)
        m_PartitionTable = new PartitionTable(*other.m_PartitionTable);
    if (other.m_SmartStatus)
        m_SmartStatus = new SmartStatus(*other.m_SmartStatus);
}

/** Destructs a Device. */
Device::~Device()
{
    delete m_PartitionTable;
}

bool Device::operator==(const Device& other) const
{
    return m_DeviceNode == other.m_DeviceNode;
}

bool Device::operator!=(const Device& other) const
{
    return !(other == *this);
}

QString Device::prettyName() const
{
    return xi18nc("@item:inlistbox Device name – Capacity (device node)", "%1 – %2 (%3)", name(), Capacity::formatByteSize(capacity()), deviceNode());
}
