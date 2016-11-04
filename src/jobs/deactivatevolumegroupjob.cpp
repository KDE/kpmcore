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

#include "jobs/deactivatevolumegroupjob.h"

#include "core/lvmdevice.h"
#include "core/partition.h"

#include "util/report.h"

#include <KLocalizedString>

/** Deactivate LVM Volume Group
*/
DeactivateVolumeGroupJob::DeactivateVolumeGroupJob(VolumeManagerDevice& d) :
    Job(),
    m_Device(d)
{
}

bool DeactivateVolumeGroupJob::run(Report& parent)
{
    bool rval = false;

    Report* report = jobStarted(parent);

    if (device().type() == Device::LVM_Device) {
        rval = LvmDevice::deactivateVG(*report, static_cast<LvmDevice&>(device()));
    }
    const auto lvmPVs = static_cast<LvmDevice&>(device()).physicalVolumes();
    for (auto &p : lvmPVs) {
        Partition *partition = const_cast<Partition *>(p);
        partition->setMounted(false);
    }

    jobFinished(*report, rval);

    return rval;
}

QString DeactivateVolumeGroupJob::description() const
{
    return xi18nc("@info/plain", "Deactivate Volume Group: <filename>%1</filename>", device().name());
}
