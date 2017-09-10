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

#if !defined(KPMCORE_RESIZEVOLUMEGROUPOPERATION_H)

#define KPMCORE_RESIZEVOLUMEGROUPOPERATION_H

#include "util/libpartitionmanagerexport.h"

#include "ops/operation.h"

#include "core/lvmdevice.h"

#include <QString>

class ResizeVolumeGroupJob;
class MovePhysicalVolumeJob;
class OperationStack;
class LvmDevice;

class LIBKPMCORE_EXPORT ResizeVolumeGroupOperation : public Operation
{
    Q_DISABLE_COPY(ResizeVolumeGroupOperation)

    friend class OperationStack;

public:
    ResizeVolumeGroupOperation(LvmDevice& dev, const QVector<const Partition*>& partlist);

public:
    QString iconName() const override {
        return QStringLiteral("arrow-right-double");
    }

    QString description() const override;

    virtual bool targets(const Device&) const override;
    virtual bool targets(const Partition&) const override;

    virtual void preview() override;
    virtual void undo() override;

    QStringList getToRemoveList();
    QStringList getToInsertList();

protected:
    LvmDevice& device() {
        return m_Device;
    }
    const LvmDevice& device() const {
        return m_Device;
    }
    const QVector<const Partition*>& targetList() const {
        return m_TargetList;
    }

    const QVector<const Partition*>& currentList() const {
        return m_CurrentList;
    }

    qint64 targetSize() const {
        return m_TargetSize;
    }

    qint64 currentSize() const {
        return m_CurrentSize;
    }

    ResizeVolumeGroupJob* growVolumeGroupJob() {
        return m_GrowVolumeGroupJob;
    }

    ResizeVolumeGroupJob* shrinkvolumegroupjob() {
        return m_ShrinkVolumeGroupJob;
    }

    MovePhysicalVolumeJob* movePhysicalVolumeJob() {
        return m_MovePhysicalVolumeJob;
    }

private:
    LvmDevice& m_Device;

    QVector<const Partition*> m_TargetList;
    QVector<const Partition*> m_CurrentList;
    qint64 m_TargetSize;
    qint64 m_CurrentSize;

    ResizeVolumeGroupJob *m_GrowVolumeGroupJob;
    ResizeVolumeGroupJob *m_ShrinkVolumeGroupJob;
    MovePhysicalVolumeJob *m_MovePhysicalVolumeJob;
};

#endif
