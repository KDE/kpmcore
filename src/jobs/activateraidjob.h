/*
    SPDX-FileCopyrightText: 2018 Caio Jordão Carvalho <caiojcarvalho@gmail.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KPMCORE_ACTIVATERAIDJOB_H
#define KPMCORE_ACTIVATERAIDJOB_H

#include "jobs/job.h"

class Report;
class QString;

class ActivateRaidJob : public Job
{
public:
    ActivateRaidJob(const QString& path);
    
public:
    bool run(Report& parent) override;
    QString description() const override;
    
private:
    const QString& m_path;
};

#endif // KPMCORE_ACTIVATERAIDJOB_H
