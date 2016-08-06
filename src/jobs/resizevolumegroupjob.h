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

#if !defined(RESIZEVOLUMEGROUPJOB_H)

#define RESIZEVOLUMEGROUPJOB_H

#include "jobs/job.h"

class LvmDevice;
class Report;

class QString;

class ResizeVolumeGroupJob : public Job
{

public:
    enum Type {
        Grow = 0,
        Shrink = 1
    };

public:
    ResizeVolumeGroupJob(LvmDevice& dev, const QStringList partlist, const Type type);

public:
    bool run(Report& parent) override;
    QString description() const override;

protected:
    LvmDevice& device() {
        return m_Device;
    }
    const LvmDevice& device() const {
        return m_Device;
    }

    const QStringList partList() const {
        return m_PartList;
    }

    ResizeVolumeGroupJob::Type type() const {
        return m_Type;
    }

private:
    LvmDevice& m_Device;
    const QStringList m_PartList;
    ResizeVolumeGroupJob::Type m_Type;
};

#endif
