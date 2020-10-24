/*
    SPDX-FileCopyrightText: 2017-2018 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2019 Yuri Chornoivan <yurchor@ukr.net>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef SFDISKDEVICE__H
#define SFDISKDEVICE__H

#include "backend/corebackenddevice.h"
#include "core/device.h"

#include <QtGlobal>

class Partition;
class PartitionTable;
class Report;
class CoreBackendPartitionTable;

class SfdiskDevice : public CoreBackendDevice
{
    Q_DISABLE_COPY(SfdiskDevice)

public:
    explicit SfdiskDevice(const Device& d);
    ~SfdiskDevice();

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
