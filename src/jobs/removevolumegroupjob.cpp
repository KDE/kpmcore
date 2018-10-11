/*************************************************************************
 *  Copyright (C) 2016 by Chantara Tith <tith.chantara@gmail.com>        *
 *  Copyright (C) 2018 by Caio Carvalho <caiojcarvalho@gmail.com>        *
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

#include "jobs/removevolumegroupjob.h"

#include "core/lvmdevice.h"
#include "core/raid/softwareraid.h"

#include "util/report.h"

#include <KLocalizedString>

/** Creates a new RemoveVolumeGroupJob
*/
RemoveVolumeGroupJob::RemoveVolumeGroupJob(VolumeManagerDevice& d) :
    Job(),
    m_Device(d)
{
}

bool RemoveVolumeGroupJob::run(Report& parent)
{
    bool rval = false;

    Report* report = jobStarted(parent);

    if (device().type() == Device::Type::LVM_Device)
        rval = LvmDevice::removeVG(*report, dynamic_cast<LvmDevice&>(device()));
    else if (device().type() == Device::Type::SoftwareRAID_Device)
        rval = SoftwareRAID::deleteSoftwareRAID(*report, dynamic_cast<SoftwareRAID&>(device()));

    jobFinished(*report, rval);

    return rval;
}

QString RemoveVolumeGroupJob::description() const
{
    return xi18nc("@info/plain", "Remove %1 Volume Group: <filename>%2</filename>", (device().type() == Device::Type::SoftwareRAID_Device ? QStringLiteral("RAID") : QStringLiteral("LVM")), device().name());
}
