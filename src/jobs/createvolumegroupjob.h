/*
    SPDX-FileCopyrightText: 2016 Chantara Tith <tith.chantara@gmail.com>
    SPDX-FileCopyrightText: 2016-2017 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2018 Caio Jordão Carvalho <caiojcarvalho@gmail.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KPMCORE_CREATEVOLUMEGROUPJOB_H
#define KPMCORE_CREATEVOLUMEGROUPJOB_H

#include "core/partition.h"
#include "core/volumemanagerdevice.h"
#include "jobs/job.h"

#include <QVector>

class LvmDevice;
class Report;

class QString;

class CreateVolumeGroupJob : public Job
{
public:
    CreateVolumeGroupJob(const QString& vgName, const QVector<const Partition*>& pvList,
                         const Device::Type type, const qint32 peSize);

    CreateVolumeGroupJob(const QString &vgName, const QVector<const Partition *> &pvList,
                         const Device::Type type, const qint32 raidLevel, const qint32 chunkSize);

public:
    bool run(Report& parent) override;
    QString description() const override;

protected:
    const QString vgName() const {
        return m_vgName;
    }

    const QVector<const Partition*>& pvList() const {
        return m_pvList;
    }

    qint32 peSize() const {
        return m_PESize;
    }

    Device::Type type() const {
        return m_type;
    }

    qint32 raidLevel() const {
        return m_raidLevel;
    }

    qint32 chunkSize() const {
        return m_chunkSize;
    }

private:
    QString m_vgName;
    QVector<const Partition*> m_pvList;
    Device::Type m_type;
    qint32 m_PESize;
    qint32 m_raidLevel;
    qint32 m_chunkSize;
};

#endif
