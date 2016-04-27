/*************************************************************************
 *  Copyright (C) 2010, 2012 by Volker Lanz <vl@fidra.de>                *
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

#if !defined(DUMMYPARTITIONTABLE__H)

#define DUMMYPARTITIONTABLE__H

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
    virtual bool open() override;

    virtual bool commit(quint32 timeout = 10) override;

    virtual CoreBackendPartition* getExtendedPartition() override;
    virtual CoreBackendPartition* getPartitionBySector(qint64 sector) override;

    virtual QString createPartition(Report& report, const Partition& partition) override;
    virtual bool deletePartition(Report& report, const Partition& partition) override;
    virtual bool updateGeometry(Report& report, const Partition& partition, qint64 sector_start, qint64 sector_end) override;
    virtual bool clobberFileSystem(Report& report, const Partition& partition) override;
    virtual bool resizeFileSystem(Report& report, const Partition& partition, qint64 newLength) override;
    virtual FileSystem::Type detectFileSystemBySector(Report& report, const Device& device, qint64 sector) override;
    virtual bool setPartitionSystemType(Report& report, const Partition& partition) override;
};

#endif
