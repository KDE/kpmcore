/*
    SPDX-FileCopyrightText: 2016 Chantara Tith <tith.chantara@gmail.com>
    SPDX-FileCopyrightText: 2019 Yuri Chornoivan <yurchor@ukr.net>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KPMCORE_DEACTIVATEVOLUMEGROUPJOB_H
#define KPMCORE_DEACTIVATEVOLUMEGROUPJOB_H

#include "jobs/job.h"

class VolumeManagerDevice;
class Partition;
class Report;

class QString;

class DeactivateVolumeGroupJob : public Job
{
public:
    explicit DeactivateVolumeGroupJob(VolumeManagerDevice& dev);

public:
    bool run(Report& parent) override;
    QString description() const override;

protected:
    VolumeManagerDevice& device() {
        return m_Device;
    }
    const VolumeManagerDevice& device() const {
        return m_Device;
    }

private:
    VolumeManagerDevice& m_Device;
};

#endif
