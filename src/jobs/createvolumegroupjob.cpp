/*
    SPDX-FileCopyrightText: 2016 Chantara Tith <tith.chantara@gmail.com>
    SPDX-FileCopyrightText: 2016-2017 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2018 Caio Jordão Carvalho <caiojcarvalho@gmail.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "jobs/createvolumegroupjob.h"

#include "core/lvmdevice.h"
#include "core/raid/softwareraid.h"

#include "util/report.h"

#include <KLocalizedString>

/** Creates a new CreateVolumeGroupJob
 * @param vgName LVM Volume Group name
 * @param pvList List of LVM Physical Volumes used to create Volume Group
 * @param peSize LVM Physical Extent size in MiB
*/
CreateVolumeGroupJob::CreateVolumeGroupJob(const QString& vgName, const QVector<const Partition*>& pvList,
                                           const Device::Type type, const qint32 peSize)
    : Job()
    , m_vgName(vgName)
    , m_pvList(pvList)
    , m_type(type)
    , m_PESize(peSize)
    , m_raidLevel(-1)
    , m_chunkSize(-1)
{
}

CreateVolumeGroupJob::CreateVolumeGroupJob(const QString& vgName, const QVector<const Partition *>& pvList,
                                           const Device::Type type, const qint32 raidLevel, const qint32 chunkSize)
    : Job()
    , m_vgName(vgName)
    , m_pvList(pvList)
    , m_type(type)
    , m_PESize(-1)
    , m_raidLevel(raidLevel)
    , m_chunkSize(chunkSize)
{
}

bool CreateVolumeGroupJob::run(Report& parent)
{
    bool rval = false;

    Report* report = jobStarted(parent);

    if (type() == Device::Type::LVM_Device)
        rval = LvmDevice::createVG(*report, vgName(), pvList(), peSize());
    else if (type() == Device::Type::SoftwareRAID_Device) {
        QStringList pathList;

        for (auto partition : pvList())
            pathList << partition->partitionPath();

        rval = SoftwareRAID::createSoftwareRAID(*report, vgName(), pathList, raidLevel(), chunkSize());
    }

    jobFinished(*report, rval);

    return rval;
}

QString CreateVolumeGroupJob::description() const
{
    QString tmp;

    for (const auto &p : pvList())
        tmp += p->deviceNode() + QStringLiteral(", ");

    tmp.chop(2);

    return xi18nc("@info/plain", "Create a new Volume Group: <filename>%1</filename> with PV: %2", vgName(), tmp);
}
