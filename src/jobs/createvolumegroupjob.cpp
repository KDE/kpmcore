/*
    SPDX-FileCopyrightText: 2016 Chantara Tith <tith.chantara@gmail.com>
    SPDX-FileCopyrightText: 2016-2017 Andrius Štikonas <andrius@stikonas.eu>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "jobs/createvolumegroupjob.h"

#include "core/lvmdevice.h"

#include "util/report.h"

#include <KLocalizedString>

/** Creates a new CreateVolumeGroupJob
 * @param vgName LVM Volume Group name
 * @param pvList List of LVM Physical Volumes used to create Volume Group
 * @param peSize LVM Physical Extent size in MiB
*/
CreateVolumeGroupJob::CreateVolumeGroupJob(const QString& vgName, const QVector<const Partition*>& pvList, const qint32 peSize) :
    Job(),
    m_vgName(vgName),
    m_pvList(pvList),
    m_PESize(peSize)
{
}

bool CreateVolumeGroupJob::run(Report& parent)
{
    bool rval = false;

    Report* report = jobStarted(parent);

    rval = LvmDevice::createVG(*report, vgName(), pvList(), peSize());

    jobFinished(*report, rval);

    return rval;
}

QString CreateVolumeGroupJob::description() const
{
    QString tmp = QString();
    for (const auto &p : pvList()) {
        tmp += p->deviceNode() + QStringLiteral(", ");
    }
    tmp.chop(2);
    return xi18nc("@info/plain", "Create a new Volume Group: <filename>%1</filename> with PV: %2", vgName(), tmp);
}
