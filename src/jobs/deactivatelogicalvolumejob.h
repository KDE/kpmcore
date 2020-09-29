/*
    SPDX-FileCopyrightText: 2016 Chantara Tith <tith.chantara@gmail.com>
    SPDX-FileCopyrightText: 2019 Yuri Chornoivan <yurchor@ukr.net>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KPMCORE_DEACTIVATELOGICALVOLUMEJOB_H
#define KPMCORE_DEACTIVATELOGICALVOLUMEJOB_H

#include "jobs/job.h"

class VolumeManagerDevice;
class Partition;
class Report;

class QString;

class DeactivateLogicalVolumeJob : public Job
{
public:
    explicit DeactivateLogicalVolumeJob(const VolumeManagerDevice& dev, const QStringList lvPaths = {});

public:
    bool run(Report& parent) override;
    QString description() const override;

protected:
    const VolumeManagerDevice& device() const {
        return m_Device;
    }

    QStringList LVList() const {
        return m_LVList;
    }

private:
    const VolumeManagerDevice& m_Device;
    const QStringList m_LVList;
};

#endif
