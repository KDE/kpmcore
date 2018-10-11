/*************************************************************************
 *  Copyright (C) 2018 by Caio Carvalho <caiojcarvalho@gmail.com>        *
 *                                                                       *
 *  This program is free software; you can redistribute it and/or        *
 *  modify it under the terms of the GNU General Public License as       *
 *  published by the Free Software Foundation; either version 3 of       *
 *  the License, or (at your option) any later version.                  *
 *                                                                       *
 *  This program is distributed in the hope that it will be useful,      *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of       *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        *
 *  GNU General Public License for more details.                         *
 *                                                                       *
 *  You should have received a copy of the GNU General Public License    *
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.*
 *************************************************************************/

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
