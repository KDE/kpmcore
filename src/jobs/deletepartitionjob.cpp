/*
    SPDX-FileCopyrightText: 2008-2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2014-2018 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2016 Chantara Tith <tith.chantara@gmail.com>
    SPDX-FileCopyrightText: 2018 Caio Jordão Carvalho <caiojcarvalho@gmail.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "jobs/deletepartitionjob.h"

#include "backend/corebackend.h"
#include "backend/corebackendmanager.h"
#include "backend/corebackenddevice.h"
#include "backend/corebackendpartitiontable.h"

#include "core/partition.h"
#include "core/device.h"
#include "core/lvmdevice.h"

#include "util/report.h"

#include <QDebug>

#include <KLocalizedString>

/** Creates a new DeletePartitionJob
    @param d the Device the Partition to delete is on
    @param p the Partition to delete
*/
DeletePartitionJob::DeletePartitionJob(Device& d, Partition& p) :
    Job(),
    m_Device(d),
    m_Partition(p)
{
}

bool DeletePartitionJob::run(Report& parent)
{
    Q_ASSERT(device().deviceNode() == partition().devicePath());

    if (device().deviceNode() != partition().devicePath()) {
        qWarning() << "deviceNode: " << device().deviceNode() << ", partition path: " << partition().devicePath();
        return false;
    }

    bool rval = false;

    Report* report = jobStarted(parent);

    if (device().type() == Device::Type::Disk_Device || device().type() == Device::Type::SoftwareRAID_Device) {
        std::unique_ptr<CoreBackendDevice> backendDevice = CoreBackendManager::self()->backend()->openDevice(device());

        if (backendDevice) {
            std::unique_ptr<CoreBackendPartitionTable> backendPartitionTable = backendDevice->openPartitionTable();

            if (backendPartitionTable) {
                rval = backendPartitionTable->deletePartition(*report, partition());

                if (!rval)
                    report->line() << xi18nc("@info:progress", "Could not delete partition <filename>%1</filename>.", partition().deviceNode());
                else
                    backendPartitionTable->commit();
            } else
                report->line() << xi18nc("@info:progress", "Could not open partition table on device <filename>%1</filename> to delete partition <filename>%2</filename>.", device().deviceNode(), partition().deviceNode());
        } else
            report->line() << xi18nc("@info:progress", "Deleting partition failed: Could not open device <filename>%1</filename>.", device().deviceNode());
    } else if (device().type() == Device::Type::LVM_Device) {
        LvmDevice& dev = dynamic_cast<LvmDevice&>(device());
        rval = LvmDevice::removeLV(*report, dev, partition());
    }

    jobFinished(*report, rval);

    return rval;
}

QString DeletePartitionJob::description() const
{
    return xi18nc("@info:progress", "Delete the partition <filename>%1</filename>", partition().deviceNode());
}
