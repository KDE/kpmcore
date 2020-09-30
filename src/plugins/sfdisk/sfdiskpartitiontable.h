/*
    SPDX-FileCopyrightText: 2017 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2019 Yuri Chornoivan <yurchor@ukr.net>
    SPDX-FileCopyrightText: 2020 Gaël PORTAY <gael.portay@collabora.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef SFDISKPARTITIONTABLE__H
#define SFDISKPARTITIONTABLE__H

#include "backend/corebackendpartitiontable.h"

#include "fs/filesystem.h"

#include <QtGlobal>

class CoreBackendPartition;
class Report;
class Partition;

class SfdiskPartitionTable : public CoreBackendPartitionTable
{
public:
    explicit SfdiskPartitionTable(const Device *d);
    ~SfdiskPartitionTable();

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
    bool setFlag(Report& report, const Partition& partition, PartitionTable::Flag flag, bool state) override;

private:
    const Device *m_device;
};

#endif
