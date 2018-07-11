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
