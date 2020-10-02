/*
    SPDX-FileCopyrightText: 2018 Andrius Štikonas <andrius@stikonas.eu>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

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
