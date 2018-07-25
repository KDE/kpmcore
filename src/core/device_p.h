/*************************************************************************
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

#ifndef KPMCORE_DEVICE_P_H
#define KPMCORE_DEVICE_P_H

#include "core/device.h"

#include <QString>

#include <memory>

class PartitionTable;
class SmartStatus;

class DevicePrivate
{
public:
    QString m_Name;
    QString m_DeviceNode;
    qint64  m_LogicalSectorSize;
    qint64  m_TotalLogical;
    PartitionTable* m_PartitionTable;
    QString m_IconName;
    std::shared_ptr<SmartStatus> m_SmartStatus;
    Device::Type m_Type;
};

#endif
