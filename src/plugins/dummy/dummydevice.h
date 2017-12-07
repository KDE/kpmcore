/*************************************************************************
 *  Copyright (C) 2010 by Volker Lanz <vl@fidra.de>                      *
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

#if !defined(KPMCORE_DUMMYDEVICE_H)

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
    DummyDevice(const QString& deviceNode);
    ~DummyDevice();

public:
    bool open() override;
    bool openExclusive() override;
    bool close() override;

    CoreBackendPartitionTable* openPartitionTable() override;

    bool createPartitionTable(Report& report, const PartitionTable& ptable) override;

    bool readData(QByteArray& buffer, qint64 offset, qint64 size) override;
    bool writeData(QByteArray& buffer, qint64 offset) override;
};

#endif
