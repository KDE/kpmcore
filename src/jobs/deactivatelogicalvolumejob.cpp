/*
    SPDX-FileCopyrightText: 2016 Chantara Tith <tith.chantara@gmail.com>
    SPDX-FileCopyrightText: 2016-2018 Andrius Štikonas <andrius@stikonas.eu>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

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

    if (device().type() == Device::Type::LVM_Device) {
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
