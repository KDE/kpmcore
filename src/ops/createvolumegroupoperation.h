/*
    SPDX-FileCopyrightText: 2016 Chantara Tith <tith.chantara@gmail.com>
    SPDX-FileCopyrightText: 2016-2017 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2018 Caio Jordão Carvalho <caiojcarvalho@gmail.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KPMCORE_CREATEVOLUMEGROUPOPERATION_H
#define KPMCORE_CREATEVOLUMEGROUPOPERATION_H

#include "util/libpartitionmanagerexport.h"

#include "ops/operation.h"

#include "core/lvmdevice.h"
#include "core/volumemanagerdevice.h"

#include <QString>

class CreateVolumeGroupJob;
class OperationStack;

class LIBKPMCORE_EXPORT CreateVolumeGroupOperation : public Operation
{
    Q_DISABLE_COPY(CreateVolumeGroupOperation)

    friend class OperationStack;

public:
    CreateVolumeGroupOperation(const QString& vgName, const QVector<const Partition*>& pvList,
                               const Device::Type type, const qint32 peSize = 4);

    CreateVolumeGroupOperation(const QString& vgName, const QVector<const Partition*>& pvList,
                               const Device::Type type, const qint32 raidLevel,
                               const qint32 chunkSize);

public:
    QString iconName() const override {
        return QStringLiteral("document-new");
    }

    QString description() const override;

    virtual bool targets(const Device&) const override {
        return true;
    }
    virtual bool targets(const Partition&) const override;

    virtual void preview() override;
    virtual void undo() override;

    static bool canCreate();

protected:
    CreateVolumeGroupJob* createVolumeGroupJob() const {
        return m_CreateVolumeGroupJob;
    }

    const QVector<const Partition*>& PVList() const {
        return m_PVList;
    }

    Device::Type type() const {
        return m_type;
    }

private:
    CreateVolumeGroupJob* m_CreateVolumeGroupJob;
    const QVector<const Partition*> m_PVList;
    QString m_vgName;
    Device::Type m_type;
};

#endif
