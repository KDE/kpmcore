/*
    SPDX-FileCopyrightText: 2016 Chantara Tith <tith.chantara@gmail.com>
    SPDX-FileCopyrightText: 2016-2018 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2018 Caio Jordão Carvalho <caiojcarvalho@gmail.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

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
