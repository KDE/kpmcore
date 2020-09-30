/*
    SPDX-FileCopyrightText: 2016 Chantara Tith <tith.chantara@gmail.com>
    SPDX-FileCopyrightText: 2016-2017 Andrius Štikonas <andrius@stikonas.eu>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KPMCORE_RESIZEVOLUMEGROUPOPERATION_H
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
