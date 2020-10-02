/*
    SPDX-FileCopyrightText: 2016 Chantara Tith <tith.chantara@gmail.com>
    SPDX-FileCopyrightText: 2016 Andrius Štikonas <andrius@stikonas.eu>

    SPDX-License-Identifier: GPL-3.0-or-later
*/


#ifndef KPMCORE_MOVEPHYSICALVOLUMEJOB_H
#define KPMCORE_MOVEPHYSICALVOLUMEJOB_H

#include "core/partition.h"
#include "jobs/job.h"

class VolumeManagerDevice;
class Report;

class QString;

class MovePhysicalVolumeJob : public Job
{
public:
    MovePhysicalVolumeJob(VolumeManagerDevice& dev, const QList <const Partition*>& partlist);

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
    const QList <const Partition*>& partList() const {
        return m_PartList;
    }

private:
    VolumeManagerDevice& m_Device;
    const QList <const Partition*> m_PartList;
};

#endif
