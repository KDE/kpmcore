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

#include "ops/resizevolumegroupoperation.h"

#include "core/lvmdevice.h"
#include "fs/lvm2_pv.h"
#include "core/partition.h"

#include "jobs/resizevolumegroupjob.h"
#include "jobs/movephysicalvolumejob.h"

#include <QString>

#include <KLocalizedString>

/** Creates a new ResizeVolumeGroupOperation.
    @param d the Device to create the new PartitionTable on
    @param t the type for the new PartitionTable
*/
ResizeVolumeGroupOperation::ResizeVolumeGroupOperation(LvmDevice& dev, const QStringList partlist) :
    Operation(),
    m_Device(dev),
    m_TargetList(partlist),
    m_CurrentList(LvmDevice::getPVs(dev.name())),
    m_GrowVolumeGroupJob(nullptr),
    m_ShrinkVolumeGroupJob(nullptr),
    m_MovePhysicalVolumeJob(nullptr)
{
    const QStringList clist = LvmDevice::getPVs(dev.name());

    QStringList toRemoveList = clist;
    foreach (QString path, partlist) {
        if (toRemoveList.contains(path)) {
            toRemoveList.removeAll(path);
        }
    }

    QStringList toInsertList = partlist;
    foreach (QString path, clist) {
        if (toInsertList.contains(path)) {
            toInsertList.removeAll(path);
        }
    }

    qint64 freePE = FS::lvm2_pv::getFreePE(clist) - FS::lvm2_pv::getFreePE(toRemoveList);
    qint64 movePE = FS::lvm2_pv::getAllocatedPE(toRemoveList);
    qint64 growPE = FS::lvm2_pv::getPVSize(toInsertList) / LvmDevice::getPeSize(dev.name());

    if ( movePE > (freePE + growPE)) {
        // *ABORT* can't move
    } else if (partlist == clist) {
        // *DO NOTHING*
    } else {
        if (!toInsertList.isEmpty()) {
            m_GrowVolumeGroupJob = new ResizeVolumeGroupJob(dev, toInsertList, ResizeVolumeGroupJob::Grow);
            addJob(growVolumeGroupJob());
        }
        if (!toRemoveList.isEmpty()) {
            m_MovePhysicalVolumeJob = new MovePhysicalVolumeJob(dev, toRemoveList);
            m_ShrinkVolumeGroupJob = new ResizeVolumeGroupJob(dev, toRemoveList, ResizeVolumeGroupJob::Shrink);
            addJob(movePhysicalVolumeJob());
            addJob(shrinkvolumegroupjob());
        }

    }

}

QString ResizeVolumeGroupOperation::description() const
{
    QString tlist = QString();
    foreach (QString path, targetList()) {
        tlist += path + QStringLiteral(",");
    }
    QString clist = QString();
    foreach (QString path, currentList()) {
        clist += path + QStringLiteral(",");
    }
    return xi18nc("@info/plain", "Resize volume %1 From\n%2 To\n%3", device().name(), clist, tlist);
}

bool ResizeVolumeGroupOperation::targets(const Device& d) const
{
    return d == device();
}

bool ResizeVolumeGroupOperation::targets(const Partition& part) const
{
    foreach (QString partPath, targetList()) {
        if (partPath == part.partitionPath()) {
            return true;
        }
    }
    return false;
}

void ResizeVolumeGroupOperation::preview()
{
}

void ResizeVolumeGroupOperation::undo()
{
}
