/*
    SPDX-FileCopyrightText: 2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2014-2018 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2015 Teo Mrnjavac <teo@kde.org>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "plugins/dummy/dummydevice.h"
#include "plugins/dummy/dummypartitiontable.h"

#include "core/partitiontable.h"

#include "util/globallog.h"
#include "util/report.h"

DummyDevice::DummyDevice(const QString& deviceNode) :
    CoreBackendDevice(deviceNode)
{
}

DummyDevice::~DummyDevice()
{
}

bool DummyDevice::open()
{
    return true;
}

bool DummyDevice::openExclusive()
{
    return true;
}

bool DummyDevice::close()
{
    return true;
}

std::unique_ptr<CoreBackendPartitionTable> DummyDevice::openPartitionTable()
{
    return std::make_unique<DummyPartitionTable>(DummyPartitionTable());
}

bool DummyDevice::createPartitionTable(Report& report, const PartitionTable& ptable)
{
    Q_UNUSED(report)
    Q_UNUSED(ptable)

    return true;
}
