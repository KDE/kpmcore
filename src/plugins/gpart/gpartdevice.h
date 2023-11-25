/*
    SPDX-FileCopyrightText: 2023 Er2 <er2@dismail.de>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KPMCORE_GPARTDEVICE_H
#define KPMCORE_GPARTDEVICE_H

#include "backend/corebackenddevice.h"
#include "core/device.h"

#include <QtGlobal>

class Partition;
class PartitionTable;
class Report;
class CoreBackendPartitionTable;

class GpartDevice : public CoreBackendDevice
{
    Q_DISABLE_COPY(GpartDevice)

public:
    explicit GpartDevice(const Device& d);
    ~GpartDevice() = default;

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
