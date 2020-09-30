/*
    SPDX-FileCopyrightText: 2016 Chantara Tith <tith.chantara@gmail.com>
    SPDX-FileCopyrightText: 2016 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2019 Yuri Chornoivan <yurchor@ukr.net>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KPMCORE_DEACTIVATEVOLUMEGROUPOPERATION_H
#define KPMCORE_DEACTIVATEVOLUMEGROUPOPERATION_H

#include "util/libpartitionmanagerexport.h"

#include "ops/operation.h"

#include <QString>

class DeactivateLogicalVolumeJob;
class DeactivateVolumeGroupJob;
class VolumeManagerDevice;
class OperationStack;
class PartitionTable;

class LIBKPMCORE_EXPORT DeactivateVolumeGroupOperation : public Operation
{
    Q_DISABLE_COPY(DeactivateVolumeGroupOperation)

    friend class OperationStack;

public:
    explicit DeactivateVolumeGroupOperation(VolumeManagerDevice& d);

public:
    QString iconName() const override {
        return QStringLiteral("edit-delete");
    }

    QString description() const override;

    virtual bool targets(const Device&) const override {
        return true;
    }
    virtual bool targets(const Partition&) const override {
        return false;
    }

    virtual void preview() override;
    virtual void undo() override;

    static bool isDeactivatable(const VolumeManagerDevice* dev);

protected:
    DeactivateVolumeGroupJob* deactivateVolumeGroupJob() {
        return m_DeactivateVolumeGroupJob;
    }

    DeactivateLogicalVolumeJob* deactivateLogicalVolumeJob() {
        return m_DeactivateLogicalVolumeJob;
    }

    VolumeManagerDevice& device() {
        return m_Device;
    }

private:
    DeactivateVolumeGroupJob* m_DeactivateVolumeGroupJob;
    DeactivateLogicalVolumeJob* m_DeactivateLogicalVolumeJob;
    VolumeManagerDevice& m_Device;
    PartitionTable* m_PartitionTable;
};

#endif
