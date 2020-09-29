/*
    SPDX-FileCopyrightText: 2008-2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2016 Andrius Štikonas <andrius@stikonas.eu>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KPMCORE_SETPARTFLAGSJOB_H
#define KPMCORE_SETPARTFLAGSJOB_H

#include "jobs/job.h"

#include "core/partitiontable.h"

class Device;
class Partition;
class Report;

class QString;

/** Set a Partition's flags.

    Set the Partition flags for a given Partition on a given Device.

    @author Volker Lanz <vl@fidra.de>
*/
class SetPartFlagsJob : public Job
{
public:
    SetPartFlagsJob(Device& d, Partition& p, PartitionTable::Flags flags);

public:
    bool run(Report& parent) override;
    qint32 numSteps() const override;
    QString description() const override;

protected:
    Device& device() {
        return m_Device;
    }
    const Device& device() const {
        return m_Device;
    }

    Partition& partition() {
        return m_Partition;
    }
    const Partition& partition() const {
        return m_Partition;
    }

    PartitionTable::Flags flags() const {
        return m_Flags;
    }

private:
    Device& m_Device;
    Partition& m_Partition;
    PartitionTable::Flags m_Flags;
};

#endif
