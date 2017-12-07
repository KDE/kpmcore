/*************************************************************************
 *  Copyright (C) 2017 by Andrius Štikonas <andrius@stikonas.eu>         *
 *                                                                       *
 *  This program is free software; you can redistribute it and/or        *
 *  modify it under the terms of the GNU General Public License as       *
 *  published by the Free Software Foundation; either version 3 of       *
 *  the License, or (at your option) any later version.                  *
 *                                                                       *
 *  This program is distributed in the hope that it will be useful,      *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of       *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        *
 *  GNU General Public License for more details.                         *
 *                                                                       *
 *  You should have received a copy of the GNU General Public License    *
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.*
 *************************************************************************/

#if !defined(SFDISKPARTITIONTABLE__H)

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
    SfdiskPartitionTable(const Device *d);
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
    bool setPartitionSystemType(Report& report, const Partition& partition) override;
    bool setFlag(Report& report, const Partition& partition, PartitionTable::Flag flag, bool state) override;

private:
    const Device *m_device;
};

#endif
