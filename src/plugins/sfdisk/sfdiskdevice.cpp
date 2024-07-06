/*
    SPDX-FileCopyrightText: 2017-2018 Andrius Štikonas <andrius@stikonas.eu>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

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
    if (ptable.type() == PartitionTable::msdos)
        tableType = QByteArrayLiteral("dos");
    else
        tableType = ptable.typeName().toLocal8Bit();

    ExternalCommand createCommand(report, QStringLiteral("sfdisk"), { QStringLiteral("--wipe=always"), m_device->deviceNode() } );
    if ( createCommand.write(QByteArrayLiteral("label: ") + tableType +
                                    QByteArrayLiteral("\nwrite\n")) && createCommand.start(-1) ) {
        return createCommand.output().contains(QStringLiteral("Script header accepted."));
    }

    return false;
}
