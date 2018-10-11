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

#include "core/device.h"
#include "core/raid/softwareraid.h"
#include "jobs/activateraidjob.h"
#include "ops/activateraidoperation.h"

#include <KLocalizedString>

ActivateRaidOperation::ActivateRaidOperation(SoftwareRAID* raid)
    : Operation()
    , m_activateRaidJob(new ActivateRaidJob(raid->deviceNode()))
    , m_raid(raid)
{
    addJob(activateRaidJob());
}

QString ActivateRaidOperation::description() const
{
    return xi18nc("@info/plain", "Activate RAID array \'%1\'.", m_raid->deviceNode());
}

bool ActivateRaidOperation::targets(const Device& device) const
{
    return device.type() == Device::Type::SoftwareRAID_Device && 
           device.deviceNode() == m_raid->deviceNode();
}

void ActivateRaidOperation::preview()
{
    m_raid->setStatus(SoftwareRAID::Status::Active);
}

void ActivateRaidOperation::undo()
{
    m_raid->setStatus(SoftwareRAID::Status::Inactive);
}
