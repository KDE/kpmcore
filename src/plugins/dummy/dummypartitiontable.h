/*
    SPDX-FileCopyrightText: 2010-2013 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2013-2017 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2020 Gaël PORTAY <gael.portay@collabora.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KPMCORE_DUMMYPARTITIONTABLE_H
#define KPMCORE_DUMMYPARTITIONTABLE_H

#include "backend/corebackendpartitiontable.h"

#include "fs/filesystem.h"

#include <QtGlobal>

class CoreBackendPartition;
class Report;
class Partition;

class DummyPartitionTable : public CoreBackendPartitionTable
{
public:
    DummyPartitionTable();
    ~DummyPartitionTable();

public:
    bool open() override;

    bool commit(quint32 timeout = 10) override;

    QString createPartition(Report& report, const Partition& partition) override;
    bool deletePartition(Report& report, const Partition& partition) override;
    bool updateGeometry(Report& report, const Partition& partition, qint64 sector_start, qint64 sector_end) override;
    bool clobberFileSystem(Report& report, const Partition& partition) override;
    bool resizeFileSystem(Report& report, const Partition& partition, qint64 newLength) override;
    FileSystem::Type detectFileSystemBySector(Report& report, const Device& device, qint64 sector) override;
    bool setPartitionLabel(Report& report, const Partition& partition, const QString& label) override;
    QString getPartitionUUID(Report& report, const Partition& partition) override;
    bool setPartitionUUID(Report& report, const Partition& partition, const QString& uuid) override;
    bool setPartitionAttributes(Report& report, const Partition& partition, quint64 attrs) override;
    bool setPartitionSystemType(Report& report, const Partition& partition) override;
    bool setFlag(Report& report, const Partition& partition, PartitionTable::Flag partitionManagerFlag, bool state) override;
};

#endif
