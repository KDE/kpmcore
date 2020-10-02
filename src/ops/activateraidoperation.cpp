/*
    SPDX-FileCopyrightText: 2018 Caio Jordão Carvalho <caiojcarvalho@gmail.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

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
