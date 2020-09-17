/*************************************************************************
 *  Copyright (C) 2020 by Gaël PORTAY <gael.portay@collabora.com>        *
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

#if !defined(KPMCORE_SETPARTITIONATTRIBUTESJOB_H)

#define KPMCORE_SETPARTITIONATTRIBUTESJOB_H

#include "jobs/job.h"

class Partition;
class Device;
class Report;

/** Set a Partition attributes (GPT only).
    @author Gaël PORTAY <gael.portay@collabora.com>
*/
class SetPartitionAttributesJob : public Job
{
public:
    SetPartitionAttributesJob(Device& d, Partition& p, quint64 newAttrs);

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

    Device& device() {
        return m_Device;
    }
    const Device& device() const {
        return m_Device;
    }

    quint64 attributes() const {
        return m_Attributes;
    }
    void setAttributes(quint64 f) {
        m_Attributes = f;
    }

private:
    Device& m_Device;
    Partition& m_Partition;
    quint64 m_Attributes;
};

#endif
