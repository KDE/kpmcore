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

#include "jobs/createpartitionjob.h"

#include "backend/corebackend.h"
#include "backend/corebackendmanager.h"
#include "backend/corebackenddevice.h"
#include "backend/corebackendpartitiontable.h"

#include "core/partition.h"
#include "core/device.h"
#include "core/lvmdevice.h"

#include "util/report.h"

#include <KLocalizedString>

/** Creates a new CreatePartitionJob
    @param d the Device the Partition to be created will be on
    @param p the Partition to create
*/
CreatePartitionJob::CreatePartitionJob(Device& d, Partition& p) :
    Job(),
    m_Device(d),
    m_Partition(p)
{
}

bool CreatePartitionJob::run(Report& parent)
{
    Q_ASSERT(partition().devicePath() == device().deviceNode());

    bool rval = false;

    Report* report = jobStarted(parent);

    if (device().type() == Device::Type::Disk_Device || device().type() == Device::Type::SoftwareRAID_Device) {
        std::unique_ptr<CoreBackendDevice> backendDevice = CoreBackendManager::self()->backend()->openDevice(device());

        if (backendDevice) {
            std::unique_ptr<CoreBackendPartitionTable> backendPartitionTable = backendDevice->openPartitionTable();

            if (backendPartitionTable) {
                QString partitionPath = backendPartitionTable->createPartition(*report, partition());

                if (partitionPath != QString()) {
                    rval = true;
                    partition().setPartitionPath(partitionPath);
                    partition().setState(Partition::State::None);
                    backendPartitionTable->commit();
                } else
                    report->line() << xi18nc("@info/plain", "Failed to add partition <filename>%1</filename> to device <filename>%2</filename>.", partition().deviceNode(), device().deviceNode());
            } else
                report->line() << xi18nc("@info:progress", "Could not open partition table on device <filename>%1</filename> to create new partition <filename>%2</filename>.", device().deviceNode(), partition().deviceNode());
        } else
            report->line() << xi18nc("@info:progress", "Could not open device <filename>%1</filename> to create new partition <filename>%2</filename>.", device().deviceNode(), partition().deviceNode());
    } else if (device().type() == Device::Type::LVM_Device) {
        LvmDevice& dev = dynamic_cast<LvmDevice&>(device());
        partition().setState(Partition::State::None);

        QString partPath = partition().partitionPath();
        QString lvname   = partPath.right(partPath.length() - partPath.lastIndexOf(QStringLiteral("/")) - 1);
        rval = LvmDevice::createLV(*report, dev, partition(), lvname);
    }

    jobFinished(*report, rval);

    return rval;
}

QString CreatePartitionJob::description() const
{
    if (partition().number() > 0)
        return xi18nc("@info:progress", "Create new partition <filename>%1</filename>", partition().deviceNode());

    return xi18nc("@info:progress", "Create new partition on device <filename>%1</filename>", device().deviceNode());
}
