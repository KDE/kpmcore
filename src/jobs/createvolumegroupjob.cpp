/*************************************************************************
 *  Copyright (C) 2016 by Chantara Tith <tith.chantara@gmail.com>        *
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

#include "util/report.h"

#include <KLocalizedString>

/** Creates a new CreateVolumeGroupJob
    @param vgname LVM Volume Group name
    @param pvlist List of LVM Physical Volumes
*/
CreateVolumeGroupJob::CreateVolumeGroupJob(const QString& vgname, const QStringList& pvlist, const qint32 pesize) :
    Job(),
    m_vgName(vgname),
    m_pvList(pvlist),
    m_PESize(pesize)
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
    for (const auto &name : pvList()) {
        tmp += QStringLiteral("\n") + name;
    }
    return xi18nc("@info/plain", "Create new Volume Group: <filename>%1</filename> with PV: %2", vgName(), tmp);
}
