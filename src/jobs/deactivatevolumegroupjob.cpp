/*
    SPDX-FileCopyrightText: 2016 Chantara Tith <tith.chantara@gmail.com>
    SPDX-FileCopyrightText: 2016-2018 Andrius Štikonas <andrius@stikonas.eu>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

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

    if (device().type() == Device::Type::LVM_Device) {
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
