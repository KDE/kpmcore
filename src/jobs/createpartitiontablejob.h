/*
    SPDX-FileCopyrightText: 2008-2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2016 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2019 Yuri Chornoivan <yurchor@ukr.net>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KPMCORE_CREATEPARTITIONTABLEJOB_H
#define KPMCORE_CREATEPARTITIONTABLEJOB_H

#include "jobs/job.h"

class Device;
class Report;

class QString;

/** Create a PartitionTable.
    @author Volker Lanz <vl@fidra.de>
*/
class CreatePartitionTableJob : public Job
{
public:
    explicit CreatePartitionTableJob(Device& d);

public:
    bool run(Report& parent) override;
    QString description() const override;

protected:
    Device& device() {
        return m_Device;
    }
    const Device& device() const {
        return m_Device;
    }

private:
    Device& m_Device;
};

#endif
