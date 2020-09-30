/*
    SPDX-FileCopyrightText: 2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2016-2018 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2019 Yuri Chornoivan <yurchor@ukr.net>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KPMCORE_DUMMYDEVICE_H
#define KPMCORE_DUMMYDEVICE_H

#include "backend/corebackenddevice.h"

#include <QtGlobal>

class Partition;
class PartitionTable;
class Report;
class CoreBackendPartitionTable;

class DummyDevice : public CoreBackendDevice
{
    Q_DISABLE_COPY(DummyDevice)

public:
    explicit DummyDevice(const QString& deviceNode);
    ~DummyDevice();

public:
    bool open() override;
    bool openExclusive() override;
    bool close() override;

    std::unique_ptr<CoreBackendPartitionTable> openPartitionTable() override;

    bool createPartitionTable(Report& report, const PartitionTable& ptable) override;
};

#endif
