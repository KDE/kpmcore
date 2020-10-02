/*
    SPDX-FileCopyrightText: 2020 Gaël PORTAY <gael.portay@collabora.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "jobs/setpartitionuuidjob.h"

#include "backend/corebackend.h"
#include "backend/corebackendmanager.h"
#include "backend/corebackenddevice.h"
#include "backend/corebackendpartitiontable.h"

#include "core/partition.h"
#include "core/device.h"

#include "util/report.h"

#include <KLocalizedString>
#include <memory>

/** Creates a new SetPartitionUUIDJob (GPT only)
    @param d the Device the Partition to be created will be on
    @param p the Partition whose UUID is to be set is on
    @param newUUID the new UUID
*/
SetPartitionUUIDJob::SetPartitionUUIDJob(Device& d, Partition& p, const QString& newUUID) :
    Job(),
    m_Device(d),
    m_Partition(p),
    m_UUID(newUUID)
{
}

bool SetPartitionUUIDJob::run(Report& parent)
{
    Q_ASSERT(partition().devicePath() == device().deviceNode());

    bool rval = true;

    Report* report = jobStarted(parent);

    // The UUID is supported by GPT only, if the partition table is not GPT, just ignore the
    // request and say all is well. This helps in operations because we don't have to check for
    // support to avoid having a failed job.
    if (m_Device.partitionTable()->type() != PartitionTable::gpt)
        report->line() << xi18nc("@info:progress", "Partition table of partition <filename>%1</filename> does not support setting UUIDs. Job ignored.", partition().deviceNode());
    else {
        std::unique_ptr<CoreBackendDevice> backendDevice = CoreBackendManager::self()->backend()->openDevice(m_Device);
        if (backendDevice) {
            std::unique_ptr<CoreBackendPartitionTable> backendPartitionTable = backendDevice->openPartitionTable();

            if (backendPartitionTable) {
                if (backendPartitionTable->setPartitionUUID(*report, partition(), m_UUID)) {
                    rval = true;
                    partition().setUUID(m_UUID);
                    backendPartitionTable->commit();
                } else
                    report->line() << xi18nc("@info:progress", "Failed to set the UUID for the partition <filename>%1</filename>.", partition().deviceNode());
            } else
                report->line() << xi18nc("@info:progress", "Could not open partition table on device <filename>%1</filename> to set the UUID for the partition <filename>%2</filename>.", device().deviceNode(), partition().deviceNode());
        } else
                report->line() << xi18nc("@info:progress", "Could not open device <filename>%1</filename> to set the UUID for partition <filename>%2</filename>.", device().deviceNode(), partition().deviceNode());

    }

    jobFinished(*report, rval);

    return rval;
}

QString SetPartitionUUIDJob::description() const
{
    return xi18nc("@info:progress", "Set the UUID on partition <filename>%1</filename> to \"%2\"", partition().deviceNode(), uuid());
}
