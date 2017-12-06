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

#include "jobs/createpartitiontablejob.h"

#include "backend/corebackendmanager.h"
#include "backend/corebackenddevice.h"
#include "backend/corebackend.h"

#include "core/device.h"
#include "core/partitiontable.h"

#include "util/report.h"

#include <KLocalizedString>

/** Creates a new CreatePartitionTableJob
    @param d the Device a new PartitionTable is to be created on
*/
CreatePartitionTableJob::CreatePartitionTableJob(Device& d) :
    Job(),
    m_Device(d)
{
}

bool CreatePartitionTableJob::run(Report& parent)
{
    bool rval = false;

    Report* report = jobStarted(parent);

    if (device().type() == Device::Disk_Device) {
        CoreBackendDevice* backendDevice = CoreBackendManager::self()->backend()->openDevice(device());

        if (backendDevice != nullptr) {
            Q_ASSERT(device().partitionTable());

            rval = backendDevice->createPartitionTable(*report, *device().partitionTable());

            delete backendDevice;
        } else
            report->line() << xi18nc("@info:progress", "Creating partition table failed: Could not open device <filename>%1</filename>.", device().deviceNode());
    } else if (device().type() == Device::LVM_Device) {
        //TODO: figure what to do with LVM partitionTable
    }

    jobFinished(*report, rval);

    return rval;
}

QString CreatePartitionTableJob::description() const
{
    return xi18nc("@info:progress", "Create new partition table on device <filename>%1</filename>", device().deviceNode());
}
