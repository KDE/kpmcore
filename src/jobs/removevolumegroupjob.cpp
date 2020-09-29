/*
    SPDX-FileCopyrightText: 2016 Chantara Tith <tith.chantara@gmail.com>
    SPDX-FileCopyrightText: 2016-2018 Andrius Štikonas <andrius@stikonas.eu>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "jobs/removevolumegroupjob.h"

#include "core/lvmdevice.h"

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

    if (device().type() == Device::Type::LVM_Device) {
        rval = LvmDevice::removeVG(*report, dynamic_cast<LvmDevice&>(device()));
    }

    jobFinished(*report, rval);

    return rval;
}

QString RemoveVolumeGroupJob::description() const
{
    return xi18nc("@info/plain", "Remove Volume Group: <filename>%1</filename>", device().name());
}
