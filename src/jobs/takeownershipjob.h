/*
    SPDX-FileCopyrightText: 2026 Ramil Nurmanov <ramil2004nur@gmail.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KPMCORE_TAKEOWNERSHIPJOB_H
#define KPMCORE_TAKEOWNERSHIPJOB_H

#include "jobs/job.h"

#include <QString>

class Partition;
class Report;

class TakeOwnershipJob : public Job
{
public:
    TakeOwnershipJob(Partition &p, const QString &userName, bool recursive);

public:
    bool run(Report &parent) override;
    QString description() const override;

protected:
    Partition &partition() { return m_Partition; }
    const Partition &partition() const { return m_Partition; }

private:
    Partition &m_Partition;
    QString m_UserName;
    bool m_Recursive;
};

#endif
