/*
    SPDX-FileCopyrightText: 2016 Chantara Tith <tith.chantara@gmail.com>
    SPDX-FileCopyrightText: 2016 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2019 Yuri Chornoivan <yurchor@ukr.net>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KPMCORE_REMOVEVOLUMEGROUPOPERATION_H
#define KPMCORE_REMOVEVOLUMEGROUPOPERATION_H

#include "util/libpartitionmanagerexport.h"

#include "ops/operation.h"

#include <QString>

class PartitionTable;
class RemoveVolumeGroupJob;
class VolumeManagerDevice;
class OperationStack;

class LIBKPMCORE_EXPORT RemoveVolumeGroupOperation : public Operation
{
    Q_DISABLE_COPY(RemoveVolumeGroupOperation)

    friend class OperationStack;

public:
    explicit RemoveVolumeGroupOperation(VolumeManagerDevice& d);

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

    static bool isRemovable(const VolumeManagerDevice* dev);

protected:
    RemoveVolumeGroupJob* removeVolumeGroupJob() {
        return m_RemoveVolumeGroupJob;
    }

    VolumeManagerDevice& device() {
        return m_Device;
    }

private:
    RemoveVolumeGroupJob* m_RemoveVolumeGroupJob;
    VolumeManagerDevice& m_Device;
    PartitionTable* m_PartitionTable;
};

#endif
