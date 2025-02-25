/*
    SPDX-FileCopyrightText: 2023 Er2 <er2@dismail.de>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "plugins/geom/geomdevice.h"
#include "plugins/geom/geompartitiontable.h"

#include "core/partitiontable.h"

#include "util/externalcommand.h"
#include "util/report.h"

//#include <libgeom.h>

GeomDevice::GeomDevice(const Device& d) :
    CoreBackendDevice(d.deviceNode()),
    m_device(&d)
{
}

bool GeomDevice::open()
{
    return true;
}

bool GeomDevice::openExclusive()
{
    setExclusive(true);

    return true;
}

bool GeomDevice::close()
{
    if (isExclusive())
        setExclusive(false);

    openPartitionTable()->commit();

    return true;
}

std::unique_ptr<CoreBackendPartitionTable> GeomDevice::openPartitionTable()
{
    return std::make_unique<GeomPartitionTable>(m_device);
}

bool GeomDevice::createPartitionTable(Report& report, const PartitionTable& ptable)
{
    QString tableType = ptable.type() == PartitionTable::msdos
                      ? QStringLiteral("mbr")
                      : ptable.typeName();

    ExternalCommand destroyCommand(report, QStringLiteral("gpart"), {
        QStringLiteral("destroy"),
        QStringLiteral("-F"),
        m_device->deviceNode()
    } );
    ExternalCommand createCommand(report, QStringLiteral("gpart"), {
        QStringLiteral("create"),
        QStringLiteral("-s"),
        tableType,
        m_device->deviceNode()
    } );
    return destroyCommand.run(-1) && createCommand.run(-1) && createCommand.exitCode() == 0;
}
