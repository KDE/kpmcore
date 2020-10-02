/*
    SPDX-FileCopyrightText: 2010-2013 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2013-2018 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2020 Gaël PORTAY <gael.portay@collabora.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "plugins/dummy/dummypartitiontable.h"
#include "plugins/dummy/dummybackend.h"

#include "core/partition.h"
#include "core/device.h"

#include "fs/filesystem.h"

#include "util/report.h"

DummyPartitionTable::DummyPartitionTable() :
    CoreBackendPartitionTable()
{
}

DummyPartitionTable::~DummyPartitionTable()
{
}

bool DummyPartitionTable::open()
{
    return true;
}


bool DummyPartitionTable::commit(quint32 timeout)
{
    Q_UNUSED(timeout)

    return true;
}

QString DummyPartitionTable::createPartition(Report& report, const Partition& partition)
{
    Q_UNUSED(report)
    Q_UNUSED(partition)

    return QStringLiteral("dummy");
}

bool DummyPartitionTable::deletePartition(Report& report, const Partition& partition)
{
    Q_UNUSED(report)
    Q_UNUSED(partition)

    return true;
}

bool DummyPartitionTable::updateGeometry(Report& report, const Partition& partition, qint64 sector_start, qint64 sector_end)
{
    Q_UNUSED(report)
    Q_UNUSED(partition)
    Q_UNUSED(sector_start)
    Q_UNUSED(sector_end)

    return true;
}

bool DummyPartitionTable::clobberFileSystem(Report& report, const Partition& partition)
{
    Q_UNUSED(report)
    Q_UNUSED(partition)

    return true;
}

bool DummyPartitionTable::resizeFileSystem(Report& report, const Partition& partition, qint64 newLength)
{
    Q_UNUSED(report)
    Q_UNUSED(partition)
    Q_UNUSED(newLength)

    return true;
}

FileSystem::Type DummyPartitionTable::detectFileSystemBySector(Report& report, const Device& device, qint64 sector)
{
    Q_UNUSED(report)
    Q_UNUSED(device)
    Q_UNUSED(sector)

    FileSystem::Type rval = FileSystem::Type::Unknown;
    return rval;
}

bool DummyPartitionTable::setPartitionSystemType(Report& report, const Partition& partition)
{
    Q_UNUSED(report)
    Q_UNUSED(partition)

    return true;
}

bool DummyPartitionTable::setPartitionLabel(Report& report, const Partition& partition, const QString& label)
{
    Q_UNUSED(report)
    Q_UNUSED(partition)
    Q_UNUSED(label)

    return true;
}

QString DummyPartitionTable::getPartitionUUID(Report& report, const Partition& partition)
{
    Q_UNUSED(report)
    Q_UNUSED(partition)

    return QString();
}

bool DummyPartitionTable::setPartitionUUID(Report& report, const Partition& partition, const QString& uuid)
{
    Q_UNUSED(report)
    Q_UNUSED(partition)
    Q_UNUSED(uuid)

    return true;
}

bool DummyPartitionTable::setPartitionAttributes(Report& report, const Partition& partition, quint64 attrs)
{
    Q_UNUSED(report)
    Q_UNUSED(partition)
    Q_UNUSED(attrs)

    return true;
}

bool DummyPartitionTable::setFlag(Report& report, const Partition& partition, PartitionTable::Flag partitionManagerFlag, bool state)
{
    Q_UNUSED(report)
    Q_UNUSED(partition)
    Q_UNUSED(partitionManagerFlag)
    Q_UNUSED(state)

    return true;
}
