/*************************************************************************
 *  Copyright (C) 2016 by Chantara Tith <tith.chantara@gmail.com>        *
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

#include "jobs/deactivatelogicalvolumejob.h"

#include "core/lvmdevice.h"
#include "core/partition.h"
#include "core/partitiontable.h"

#include "util/report.h"

#include <KLocalizedString>

/** Creates a new DeactivateLogicalVolumeJob
*/
DeactivateLogicalVolumeJob::DeactivateLogicalVolumeJob(const VolumeManagerDevice& d, const QStringList lvPaths) :
    Job(),
    m_Device(d),
    m_LVList(lvPaths)
{
}

bool DeactivateLogicalVolumeJob::run(Report& parent)
{
    bool rval = true;

    Report* report = jobStarted(parent);

    if (device().type() == Device::LVM_Device) {
        for (const auto &p : device().partitionTable()->children()) {
            if (!p->roles().has(PartitionRole::Unallocated)) {
                if (!LvmDevice::deactivateLV(*report, *p)) {
                    rval = false;
                }
            }
        }
    }

    jobFinished(*report, rval);

    return rval;
}

QString DeactivateLogicalVolumeJob::description() const
{
    return xi18nc("@info/plain", "Deactivate Logical Volumes: <filename>%1</filename>", device().prettyDeviceNodeList());
}
