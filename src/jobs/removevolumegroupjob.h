/*
    SPDX-FileCopyrightText: 2016 Chantara Tith <tith.chantara@gmail.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KPMCORE_REMOVEVOLUMEGROUPJOB_H
#define KPMCORE_REMOVEVOLUMEGROUPJOB_H

#include "jobs/job.h"

class VolumeManagerDevice;
class Partition;
class Report;

class QString;

class RemoveVolumeGroupJob : public Job
{
public:
    RemoveVolumeGroupJob(VolumeManagerDevice& dev);

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
