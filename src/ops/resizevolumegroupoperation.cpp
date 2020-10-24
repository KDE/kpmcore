/*
    SPDX-FileCopyrightText: 2016 Chantara Tith <tith.chantara@gmail.com>
    SPDX-FileCopyrightText: 2016-2020 Andrius Štikonas <andrius@stikonas.eu>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "ops/resizevolumegroupoperation.h"

#include "core/lvmdevice.h"
#include "core/partition.h"
#include "fs/lvm2_pv.h"
#include "jobs/resizevolumegroupjob.h"
#include "jobs/movephysicalvolumejob.h"
#include "util/helpers.h"

#include <utility>

#include <QString>

#include <KLocalizedString>

/** Creates a new ResizeVolumeGroupOperation.
    @param d the Device to create the new PartitionTable on
    @param partList list of LVM Physical Volumes that should be in LVM Volume Group
*/
ResizeVolumeGroupOperation::ResizeVolumeGroupOperation(LvmDevice& d, const QVector<const Partition*>& partList)
    : Operation()
    , m_Device(d)
    , m_TargetList(partList)
    , m_CurrentList(d.physicalVolumes())
    , m_TargetSize(0)
    , m_CurrentSize(0)
    , m_GrowVolumeGroupJob(nullptr)
    , m_ShrinkVolumeGroupJob(nullptr)
    , m_MovePhysicalVolumeJob(nullptr)
{
    for (const auto &p : targetList())
        m_TargetSize += p->capacity();
    for (const auto &p : currentList())
        m_CurrentSize += p->capacity();

    QList<const Partition*> toRemoveList;
    for (const auto &p : currentList())
        if (!targetList().contains(p))
            toRemoveList.append(p);

    QList<const Partition*> toInsertList;
    for (const auto &p : targetList())
        if (!currentList().contains(p))
            toInsertList.append(p);

    qint64 currentFreePE = 0;
    for (const auto &p : currentList()) {
        FS::lvm2_pv *lvm2PVFs;
        innerFS(p, lvm2PVFs);
        currentFreePE += lvm2PVFs->freePE();
    }
    qint64 removedFreePE = 0;
    for (const auto &p : std::as_const(toRemoveList)) {
        FS::lvm2_pv *lvm2PVFs;
        innerFS(p, lvm2PVFs);
        removedFreePE += lvm2PVFs->freePE();
    }
    qint64 freePE = currentFreePE - removedFreePE;
    qint64 movePE = 0;
    for (const auto &p : std::as_const(toRemoveList)) {
        FS::lvm2_pv *lvm2PVFs;
        innerFS(p, lvm2PVFs);
        movePE += lvm2PVFs->allocatedPE();
    }
    qint64 growPE = 0;
    for (const auto &p : std::as_const(toInsertList)) {
        growPE += p->capacity() / device().peSize();
    }

    if ( movePE > (freePE + growPE)) {
        // *ABORT* can't move
    } else if (partList == currentList()) {
        // *DO NOTHING*
    } else {
        if (!toInsertList.isEmpty()) {
            m_GrowVolumeGroupJob = new ResizeVolumeGroupJob(d, toInsertList, ResizeVolumeGroupJob::Type::Grow);
            addJob(growVolumeGroupJob());
        }
        if (!toRemoveList.isEmpty()) {
            m_MovePhysicalVolumeJob = new MovePhysicalVolumeJob(d, toRemoveList);
            m_ShrinkVolumeGroupJob = new ResizeVolumeGroupJob(d, toRemoveList, ResizeVolumeGroupJob::Type::Shrink);
            addJob(movePhysicalVolumeJob());
            addJob(shrinkvolumegroupjob());
        }
    }
}

QString ResizeVolumeGroupOperation::description() const
{
    QString tList = QString();
    for (const auto &p : targetList()) {
        tList += p->deviceNode() + QStringLiteral(", ");
    }
    tList.chop(2);
    QString curList = QString();
    for (const auto &p : currentList()) {
        curList += p->deviceNode() + QStringLiteral(", ");
    }
    curList.chop(2);

    return xi18nc("@info/plain", "Resize volume %1 from %2 to %3", device().name(), curList, tList);
}

bool ResizeVolumeGroupOperation::targets(const Device& d) const
{
    return d == device();
}

bool ResizeVolumeGroupOperation::targets(const Partition& p) const
{
    for (const auto &partition : targetList()) {
        if (partition->partitionPath() == p.partitionPath()) {
            return true;
        }
    }
    return false;
}

void ResizeVolumeGroupOperation::preview()
{
    //assuming that targetSize is larger than the allocated space.
    device().setTotalLogical(targetSize() / device().logicalSize());
    device().partitionTable()->setFirstUsableSector(PartitionTable::defaultFirstUsable(device(), PartitionTable::vmd));
    device().partitionTable()->setLastUsableSector(PartitionTable::defaultLastUsable(device(), PartitionTable::vmd));
    device().partitionTable()->updateUnallocated(device());
}

void ResizeVolumeGroupOperation::undo()
{
    device().setTotalLogical(currentSize() / device().logicalSize());
    device().partitionTable()->setFirstUsableSector(PartitionTable::defaultFirstUsable(device(), PartitionTable::vmd));
    device().partitionTable()->setLastUsableSector(PartitionTable::defaultLastUsable(device(), PartitionTable::vmd));
    device().partitionTable()->updateUnallocated(device());
}
