/*
    SPDX-FileCopyrightText: 2018 Caio Jordão Carvalho <caiojcarvalho@gmail.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "core/raid/softwareraid.h"
#include "jobs/activateraidjob.h"
#include "util/report.h"

#include <QDebug>
#include <QString>

#include <KLocalizedString>

ActivateRaidJob::ActivateRaidJob(const QString& path)
    : m_path(path)
{
}

bool ActivateRaidJob::run(Report& parent)
{
    Report *report = jobStarted(parent);
    
    bool result = SoftwareRAID::assembleSoftwareRAID(m_path);
    
    jobFinished(*report, result);
    
    return result;
}

QString ActivateRaidJob::description() const
{
    return xi18nc("@info/plain", "Activate RAID volume: <filename>%1</filename>", m_path);
}
