/*
    SPDX-FileCopyrightText: 2008-2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2016 Andrius Štikonas <andrius@stikonas.eu>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KPMCORE_CREATEPARTITIONJOB_H
#define KPMCORE_CREATEPARTITIONJOB_H

#include "jobs/job.h"

class Partition;
class Device;
class Report;

class QString;

/** Create a Partition.
    @author Volker Lanz <vl@fidra.de>
*/
class CreatePartitionJob : public Job
{
public:
    CreatePartitionJob(Device& d, Partition& p);

public:
    bool run(Report& parent) override;
    QString description() const override;

protected:
    Partition& partition() {
        return m_Partition;
    }
    const Partition& partition() const {
        return m_Partition;
    }

    Device& device() {
        return m_Device;
    }
    const Device& device() const {
        return m_Device;
    }

private:
    Device& m_Device;
    Partition& m_Partition;
};

#endif
