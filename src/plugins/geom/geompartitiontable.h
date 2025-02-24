/*
    SPDX-FileCopyrightText: 2023 Er2 <er2@dismail.de>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KPMCORE_GEOMPARTITIONTABLE_H
#define KPMCORE_GEOMPARTITIONTABLE_H

#include "backend/corebackendpartitiontable.h"

#include "fs/filesystem.h"

#include <QtGlobal>

class CoreBackendPartition;
class Report;
class Partition;

class GeomPartitionTable : public CoreBackendPartitionTable
{
public:
    explicit GeomPartitionTable(const Device *d);
    ~GeomPartitionTable();

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

private:
    const Device *m_device;
};

#endif
