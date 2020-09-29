/*
    SPDX-FileCopyrightText: 2016 Chantara Tith <tith.chantara@gmail.com>
    SPDX-FileCopyrightText: 2016-2017 Andrius Štikonas <andrius@stikonas.eu>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KPMCORE_CREATEVOLUMEGROUPJOB_H
#define KPMCORE_CREATEVOLUMEGROUPJOB_H

#include "core/partition.h"
#include "jobs/job.h"

#include <QVector>

class LvmDevice;
class Report;

class QString;

class CreateVolumeGroupJob : public Job
{
public:
    CreateVolumeGroupJob(const QString& vgName, const QVector<const Partition*>& pvList, const qint32 peSize);

public:
    bool run(Report& parent) override;
    QString description() const override;

protected:
    QString vgName() {
        return m_vgName;
    }
    const QString vgName() const {
        return m_vgName;
    }
    QVector<const Partition*>& pvList() {
        return m_pvList;
    }
    const QVector<const Partition*>& pvList() const {
        return m_pvList;
    }

    qint32 peSize() {
        return m_PESize;
    }

private:
    QString m_vgName;
    QVector<const Partition*> m_pvList;
    qint32 m_PESize;
};

#endif
