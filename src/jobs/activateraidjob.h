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

#if !defined(KPMCORE_ACTIVATERAIDJOB_H)

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
