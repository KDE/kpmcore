/*
    SPDX-FileCopyrightText: 2026 Ramil Nurmanov <ramil2004nur@gmail.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KPMCORE_TRIMJOB_H
#define KPMCORE_TRIMJOB_H

#include "jobs/job.h"

#include <QString>

class Partition;
class Report;

class TrimJob : public Job
{
public:
    explicit TrimJob(Partition& p);

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
