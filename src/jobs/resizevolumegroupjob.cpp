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

#include "jobs/resizevolumegroupjob.h"

#include "core/lvmdevice.h"

#include "util/report.h"

#include <KLocalizedString>

/** Creates a new ResizeVolumeGroupJob
    @param vgname
    @parem pvList
*/
ResizeVolumeGroupJob::ResizeVolumeGroupJob(LvmDevice& dev, const QStringList partlist, const Type type) :
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

    //TODO:check that the provided list is legal
    foreach (QString pvpath, partList()) {
        if (type() == ResizeVolumeGroupJob::Grow) {
            rval = LvmDevice::insertPV(*report, device(), pvpath);
        } else if (type() == ResizeVolumeGroupJob::Shrink) {
            rval = LvmDevice::removePV(*report, device(), pvpath);
        }
        if (rval == false) {
            break;
        }
    }

    jobFinished(*report, rval);

    return rval;
}

QString ResizeVolumeGroupJob::description() const
{
    QString tmp = QString();
    foreach (QString path, partList()) {
        tmp += path + QStringLiteral(",");
    }
    if (type() == ResizeVolumeGroupJob::Grow) {
        return xi18nc("@info/plain", "Inserting Volume: %1 to %2.", tmp, device().name());
    }
    if (type() == ResizeVolumeGroupJob::Shrink) {
        return xi18nc("@info/plain", "Removing Volume: %1 from %2.", tmp, device().name());
    }
    return xi18nc("@info/plain", "Resizing Volume: %1 to %2.", device().name(), tmp);
}
