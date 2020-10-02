/*
    SPDX-FileCopyrightText: 2016 Chantara Tith <tith.chantara@gmail.com>
    SPDX-FileCopyrightText: 2016-2018 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2018 Caio Jordão Carvalho <caiojcarvalho@gmail.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "jobs/deactivatevolumegroupjob.h"

#include "core/lvmdevice.h"
#include "core/partition.h"
#include "core/raid/softwareraid.h"

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
        const auto lvmPVs = static_cast<LvmDevice&>(device()).physicalVolumes();
        for (auto &p : lvmPVs) {
            Partition *partition = const_cast<Partition *>(p);
            partition->setMounted(false);
        }
    }
    else if (device().type() == Device::Type::SoftwareRAID_Device) {
        rval = SoftwareRAID::stopSoftwareRAID(*report, device().deviceNode());

        if (rval) {
            SoftwareRAID *raid = static_cast< SoftwareRAID* >(&device());
            raid->setStatus(SoftwareRAID::Status::Inactive);
        }
    }

    jobFinished(*report, rval);

    return rval;
}

QString DeactivateVolumeGroupJob::description() const
{
    return xi18nc("@info/plain", "Deactivate Volume Group: <filename>%1</filename>", device().name());
}
