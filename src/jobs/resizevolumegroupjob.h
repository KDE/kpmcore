/*
    SPDX-FileCopyrightText: 2016 Chantara Tith <tith.chantara@gmail.com>
    SPDX-FileCopyrightText: 2016-2018 Andrius Štikonas <andrius@stikonas.eu>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KPMCORE_RESIZEVOLUMEGROUPJOB_H
#define KPMCORE_RESIZEVOLUMEGROUPJOB_H

#include "core/partition.h"
#include "jobs/job.h"

class LvmDevice;
class Report;

class QString;

class ResizeVolumeGroupJob : public Job
{

public:
    enum class Type {
        Grow,
        Shrink
    };

public:
    ResizeVolumeGroupJob(LvmDevice& dev, const QList <const Partition*>& partlist, const Type type);

public:
    bool run(Report& parent) override;
    QString description() const override;

protected:
    LvmDevice& device() {
        return m_Device;
    }
    const LvmDevice& device() const {
        return m_Device;
    }

    const QList <const Partition*>& partList() const {
        return m_PartList;
    }

    ResizeVolumeGroupJob::Type type() const {
        return m_Type;
    }

private:
    LvmDevice& m_Device;
    const QList <const Partition*> m_PartList;
    ResizeVolumeGroupJob::Type m_Type;
};

#endif
