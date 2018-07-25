/*************************************************************************
 *  Copyright (C) 2008 by Volker Lanz <vl@fidra.de>                      *
 *  Copyright (C) 2016 by Andrius Štikonas <andrius@stikonas.eu>         *
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
