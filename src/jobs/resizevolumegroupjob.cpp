/*
    SPDX-FileCopyrightText: 2016 Chantara Tith <tith.chantara@gmail.com>
    SPDX-FileCopyrightText: 2016-2018 Andrius Štikonas <andrius@stikonas.eu>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "jobs/resizevolumegroupjob.h"

#include "core/lvmdevice.h"
#include "fs/luks.h"

#include "util/report.h"

#include <KLocalizedString>

/** Creates a new ResizeVolumeGroupJob
*/
ResizeVolumeGroupJob::ResizeVolumeGroupJob(LvmDevice& dev, const QList <const Partition*>& partlist, const Type type) :
    Job(),
    m_Device(dev),
    m_PartList(partlist),
    m_Type(type)
{
}

bool ResizeVolumeGroupJob::run(Report& parent)
{
    bool rval = false;

    Report* report = jobStarted(parent);

    for (const auto &p : partList()) {
        const QString deviceNode = p->roles().has(PartitionRole::Luks) ? static_cast<const FS::luks*>(&p->fileSystem())->mapperName() : p->partitionPath();
        if (type() == ResizeVolumeGroupJob::Type::Grow)
            rval = LvmDevice::insertPV(*report, device(), deviceNode);
        else if (type() == ResizeVolumeGroupJob::Type::Shrink)
            rval = LvmDevice::removePV(*report, device(), deviceNode);

        if (rval == false)
            break;
    }

    jobFinished(*report, rval);

    return rval;
}

QString ResizeVolumeGroupJob::description() const
{
    QString partitionList = QString();
    for (const auto &p : partList()) {
        partitionList += p->deviceNode() + QStringLiteral(", ");
    }
    partitionList.chop(2);
    const qint32 count = partList().count();

    if (type() == ResizeVolumeGroupJob::Type::Grow) {
        return xi18ncp("@info/plain", "Adding LVM Physical Volume %2 to %3.", "Adding LVM Physical Volumes %2 to %3.", count, partitionList, device().name());
    }
    if (type() == ResizeVolumeGroupJob::Type::Shrink) {
        return xi18ncp("@info/plain", "Removing LVM Physical Volume %2 from %3.", "Removing LVM Physical Volumes %2 from %3.", count, partitionList, device().name());
    }
    return xi18nc("@info/plain", "Resizing Volume Group %1 to %2.", device().name(), partitionList);
}
