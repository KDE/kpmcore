/*************************************************************************
 *  Copyright (C) 2016 by Chantara Tith <tith.chantara@gmail.com>        *
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

#if !defined(KPMCORE_CREATEVOLUMEGROUPJOB_H)

#define KPMCORE_CREATEVOLUMEGROUPJOB_H

#include "core/partition.h"
#include "jobs/job.h"

#include <QVector>

class LvmDevice;
class Report;

class QString;

class CreateVolumeGroupJob : public Job
{
public:
    CreateVolumeGroupJob(const QString& vgName, const QVector<const Partition*>& pvList, const qint32 peSize);

public:
    bool run(Report& parent) override;
    QString description() const override;

protected:
    QString vgName() {
        return m_vgName;
    }
    const QString vgName() const {
        return m_vgName;
    }
    QVector<const Partition*>& pvList() {
        return m_pvList;
    }
    const QVector<const Partition*>& pvList() const {
        return m_pvList;
    }

    qint32 peSize() {
        return m_PESize;
    }

private:
    QString m_vgName;
    QVector<const Partition*> m_pvList;
    qint32 m_PESize;
};

#endif
