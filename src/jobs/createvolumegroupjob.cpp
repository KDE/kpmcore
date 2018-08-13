/*************************************************************************
 *  Copyright (C) 2016 by Chantara Tith <tith.chantara@gmail.com>        *
 *  Copyright (C) 2016 by Andrius Štikonas <andrius@stikonas.eu>         *
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
