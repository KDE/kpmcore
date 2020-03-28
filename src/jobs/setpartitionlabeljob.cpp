/*************************************************************************
 *  Copyright (C) 2020 by Gaël PORTAY <gael.portay@collabora.com>        *
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

#include "jobs/setpartitionlabeljob.h"

#include "backend/corebackend.h"
#include "backend/corebackendmanager.h"
#include "backend/corebackenddevice.h"
#include "backend/corebackendpartitiontable.h"

#include "core/partition.h"
#include "core/device.h"

#include "util/report.h"

#include <KLocalizedString>
#include <memory>

/** Creates a new SetPartitionLabelJob (GPT only)
    @param d the Device the Partition to be created will be on
    @param p the Partition whose label is to be set is on
    @param newLabel the new label
*/
SetPartitionLabelJob::SetPartitionLabelJob(Device& d, Partition& p, const QString& newLabel) :
    Job(),
    m_Device(d),
    m_Partition(p),
    m_Label(newLabel)
{
}

bool SetPartitionLabelJob::run(Report& parent)
{
    Q_ASSERT(partition().devicePath() == device().deviceNode());

    bool rval = true;

    Report* report = jobStarted(parent);

    // The label is supported by GPT only (as partition name), if the partition table is not GPT,
    // just ignore the request and say all is well. This helps in operations because
    // we don't have to check for support to avoid having a failed job.
    if (m_Device.partitionTable()->type() != PartitionTable::gpt)
        report->line() << xi18nc("@info:progress", "Partition table of partition <filename>%1</filename> does not support setting names. Job ignored.", partition().deviceNode());
    else {
        std::unique_ptr<CoreBackendDevice> backendDevice = CoreBackendManager::self()->backend()->openDevice(m_Device);
        if (backendDevice) {
            std::unique_ptr<CoreBackendPartitionTable> backendPartitionTable = backendDevice->openPartitionTable();

            if (backendPartitionTable) {
                if (backendPartitionTable->setPartitionLabel(*report, partition(), m_Label)) {
                    rval = true;
                    partition().setLabel(m_Label);
                    backendPartitionTable->commit();
                } else
                    report->line() << xi18nc("@info:progress", "Failed to set the system type for the file system on partition <filename>%1</filename>.", partition().deviceNode());
            } else
                report->line() << xi18nc("@info:progress", "Could not open partition table on device <filename>%1</filename> to set the system type for partition <filename>%2</filename>.", device().deviceNode(), partition().deviceNode());
        } else
                report->line() << xi18nc("@info:progress", "Could not open device <filename>%1</filename> to set the system type for partition <filename>%2</filename>.", device().deviceNode(), partition().deviceNode());

    }

    jobFinished(*report, rval);

    return rval;
}

QString SetPartitionLabelJob::description() const
{
    return xi18nc("@info:progress", "Set the label on partition <filename>%1</filename> to \"%2\"", partition().deviceNode(), label());
}
