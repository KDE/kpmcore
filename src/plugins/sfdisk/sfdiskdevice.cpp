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

#include "plugins/sfdisk/sfdiskdevice.h"
#include "plugins/sfdisk/sfdiskpartitiontable.h"

#include "core/partitiontable.h"

#include "util/externalcommand.h"
#include "util/report.h"

SfdiskDevice::SfdiskDevice(const Device& d) :
    CoreBackendDevice(d.deviceNode()),
    m_device(&d)
{
}

SfdiskDevice::~SfdiskDevice()
{
    close();
}

bool SfdiskDevice::open()
{
    return true;
}

bool SfdiskDevice::openExclusive()
{
    setExclusive(true);

    return true;
}

bool SfdiskDevice::close()
{
    if (isExclusive())
        setExclusive(false);

    CoreBackendPartitionTable* ptable = new SfdiskPartitionTable(m_device);
    ptable->commit();
    delete ptable;

    return true;
}

std::unique_ptr<CoreBackendPartitionTable> SfdiskDevice::openPartitionTable()
{
    return std::make_unique<SfdiskPartitionTable>(m_device);
}

bool SfdiskDevice::createPartitionTable(Report& report, const PartitionTable& ptable)
{
    QByteArray tableType;
    if (ptable.type() == PartitionTable::msdos || ptable.type() == PartitionTable::msdos_sectorbased)
        tableType = QByteArrayLiteral("dos");
    else
        tableType = ptable.typeName().toLocal8Bit();

    ExternalCommand createCommand(report, QStringLiteral("sfdisk"), { m_device->deviceNode() } );
    if ( createCommand.write(QByteArrayLiteral("label: ") + tableType +
                                    QByteArrayLiteral("\nwrite\n")) && createCommand.start(-1) ) {
        return createCommand.output().contains(QStringLiteral("Script header accepted."));
    }

    return false;
}
