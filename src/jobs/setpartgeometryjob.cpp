/*
    SPDX-FileCopyrightText: 2008-2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2014-2018 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2016 Chantara Tith <tith.chantara@gmail.com>
    SPDX-FileCopyrightText: 2018 Caio Jordão Carvalho <caiojcarvalho@gmail.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "jobs/setpartgeometryjob.h"

#include "backend/corebackend.h"
#include "backend/corebackendmanager.h"
#include "backend/corebackenddevice.h"
#include "backend/corebackendpartitiontable.h"

#include "core/partition.h"
#include "core/device.h"
#include "core/lvmdevice.h"

#include "util/report.h"

#include <KLocalizedString>

/** Creates a new SetPartGeometryJob
    @param d the Device the Partition whose geometry is to be set is on
    @param p the Partition whose geometry is to be set
    @param newstart the new start sector for the Partition
    @param newlength the new length for the Partition

    @todo Wouldn't it be better to have newfirst (new first sector) and newlast (new last sector) as args instead?
    Having a length here doesn't seem to be very consistent with the rest of the app, right?
*/
SetPartGeometryJob::SetPartGeometryJob(Device& d, Partition& p, qint64 newstart, qint64 newlength) :
    Job(),
    m_Device(d),
    m_Partition(p),
    m_NewStart(newstart),
    m_NewLength(newlength)
{
}

bool SetPartGeometryJob::run(Report& parent)
{
    bool rval = false;

    Report* report = jobStarted(parent);

    if(device().type() == Device::Type::Disk_Device || device().type() == Device::Type::SoftwareRAID_Device) {
        std::unique_ptr<CoreBackendDevice> backendDevice = CoreBackendManager::self()->backend()->openDevice(device());

        if (backendDevice) {
            std::unique_ptr<CoreBackendPartitionTable> backendPartitionTable = backendDevice->openPartitionTable();

            if (backendPartitionTable) {
                rval = backendPartitionTable->updateGeometry(*report, partition(), newStart(), newStart() + newLength() - 1);

                if (rval) {
                    partition().setFirstSector(newStart());
                    partition().setLastSector(newStart() + newLength() - 1);
                    backendPartitionTable->commit();
                }
            }
        } else
            report->line() << xi18nc("@info:progress", "Could not open device <filename>%1</filename> while trying to resize/move partition <filename>%2</filename>.", device().deviceNode(), partition().deviceNode());
    } else if (device().type() == Device::Type::LVM_Device) {

        partition().setFirstSector(newStart());
        partition().setLastSector(newStart() + newLength() - 1);

        rval = LvmDevice::resizeLV(*report, partition());
    }

    jobFinished(*report, rval);

    return rval;
}

QString SetPartGeometryJob::description() const
{
    return xi18nc("@info:progress", "Set geometry of partition <filename>%1</filename>: Start sector: %2, length: %3", partition().deviceNode(), newStart(), newLength());
}
