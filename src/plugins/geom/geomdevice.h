/*
    SPDX-FileCopyrightText: 2023 Er2 <er2@dismail.de>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KPMCORE_GEOMDEVICE_H
#define KPMCORE_GEOMDEVICE_H

#include "backend/corebackenddevice.h"
#include "core/device.h"

#include <QtGlobal>

class Partition;
class PartitionTable;
class Report;
class CoreBackendPartitionTable;

class GeomDevice : public CoreBackendDevice
{
    Q_DISABLE_COPY(GeomDevice)

public:
    explicit GeomDevice(const Device& d);
    ~GeomDevice() = default;

public:
    bool open() override;
    bool openExclusive() override;
    bool close() override;

    std::unique_ptr<CoreBackendPartitionTable> openPartitionTable() override;

    bool createPartitionTable(Report& report, const PartitionTable& ptable) override;

private:
    const Device *m_device;
};

#endif
