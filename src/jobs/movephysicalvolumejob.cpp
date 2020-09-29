/*
    SPDX-FileCopyrightText: 2016 Chantara Tith <tith.chantara@gmail.com>
    SPDX-FileCopyrightText: 2016 Andrius Štikonas <andrius@stikonas.eu>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "jobs/movephysicalvolumejob.h"

#include "core/lvmdevice.h"

#include "util/report.h"

#include <KLocalizedString>

/** Creates a new MovePhysicalVolumeJob
 * @param d Device representing LVM Volume Group
*/
MovePhysicalVolumeJob::MovePhysicalVolumeJob(LvmDevice& d, const QList <const Partition*>& partList) :
    Job(),
    m_Device(d),
    m_PartList(partList)
{
}

bool MovePhysicalVolumeJob::run(Report& parent)
{
    bool rval = false;

    Report* report = jobStarted(parent);

    QStringList destinations = device().deviceNodes();
    for (const auto &p : partList()) {
        if (destinations.contains(p->partitionPath())) {
            destinations.removeAll(p->partitionPath());
        }
    }

    for (const auto &p : partList()) {
        rval = LvmDevice::movePV(*report, p->partitionPath(), destinations);
        if (rval == false) {
            break;
        }
    }

    jobFinished(*report, rval);

    return rval;
}

QString MovePhysicalVolumeJob::description() const
{
    QString movedPartitions = QString();
    for (const auto &p : partList())
        movedPartitions += p->deviceNode() + QStringLiteral(", ");
    movedPartitions.chop(2);
    return xi18nc("@info/plain", "Move used PE in %1 on %2 to other available Physical Volumes", movedPartitions, device().name());
}
