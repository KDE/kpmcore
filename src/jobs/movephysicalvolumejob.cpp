/*************************************************************************
 *  Copyright (C) 2016 by Chantara Tith <tith.chantara@gmail.com>        *
 *  Copyright (C) 2016 by Andrius Štikonas <andrius@stikonas.eu>         *
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
