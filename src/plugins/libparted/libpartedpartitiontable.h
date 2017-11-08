/*************************************************************************
 *  Copyright (C) 2010, 2011 by Volker Lanz <vl@fidra.de>                *
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

#if !defined(KPMCORE_LIBPARTEDPARTITIONTABLE_H)

#define KPMCORE_LIBPARTEDPARTITIONTABLE_H

#include "backend/corebackendpartitiontable.h"

#include "fs/filesystem.h"

#include <QtGlobal>

#include <parted/parted.h>

class CoreBackendPartition;
class Report;
class Partition;

class LibPartedPartitionTable : public CoreBackendPartitionTable
{
public:
    LibPartedPartitionTable(PedDevice* device);
    ~LibPartedPartitionTable();

public:
    bool open() override;

    bool commit(quint32 timeout = 10) override;
    static bool commit(PedDisk* pd, quint32 timeout = 10);

    QString createPartition(Report& report, const Partition& partition) override;
    bool deletePartition(Report& report, const Partition& partition) override;
    bool updateGeometry(Report& report, const Partition& partition, qint64 sector_start, qint64 sector_end) override;
    bool clobberFileSystem(Report& report, const Partition& partition) override;
    bool resizeFileSystem(Report& report, const Partition& partition, qint64 newLength) override;
    FileSystem::Type detectFileSystemBySector(Report& report, const Device& device, qint64 sector) override;
    bool setPartitionSystemType(Report& report, const Partition& partition) override;
    bool setFlag(Report& report, const Partition& partition, PartitionTable::Flag flag, bool state) override;

private:
    PedDevice* pedDevice() {
        return m_PedDevice;
    }
    PedDisk* pedDisk() {
        return m_PedDisk;
    }

private:
    PedDevice* m_PedDevice;
    PedDisk* m_PedDisk;
};

#endif
