/*
    SPDX-FileCopyrightText: 2008-2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2016 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2019 Yuri Chornoivan <yurchor@ukr.net>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KPMCORE_CHECKFILESYSTEMJOB_H
#define KPMCORE_CHECKFILESYSTEMJOB_H

#include "jobs/job.h"

class Partition;
class Report;

class QString;

/** Check a FileSystem.
    @author Volker Lanz <vl@fidra.de>
*/
class CheckFileSystemJob : public Job
{
public:
    explicit CheckFileSystemJob(Partition& p);

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

private:
    Partition& m_Partition;
};

#endif
