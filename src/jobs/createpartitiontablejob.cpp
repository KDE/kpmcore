/*
    SPDX-FileCopyrightText: 2008-2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2014-2020 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2016 Chantara Tith <tith.chantara@gmail.com>
    SPDX-FileCopyrightText: 2018 Caio Jordão Carvalho <caiojcarvalho@gmail.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "jobs/createpartitiontablejob.h"

#include "backend/corebackendmanager.h"
#include "backend/corebackenddevice.h"
#include "backend/corebackend.h"

#include "core/device.h"
#include "core/partitiontable.h"

#include "util/report.h"

#include <KLocalizedString>

/** Creates a new CreatePartitionTableJob
    @param d the Device a new PartitionTable is to be created on
*/
CreatePartitionTableJob::CreatePartitionTableJob(Device& d) :
    Job(),
    m_Device(d)
{
}

bool CreatePartitionTableJob::run(Report& parent)
{
    bool rval = false;

    Report* report = jobStarted(parent);

    if (device().partitionTable()->type() == PartitionTable::TableType::none)
        return true;

    if (device().type() == Device::Type::Disk_Device || device().type() == Device::Type::SoftwareRAID_Device) {
        std::unique_ptr<CoreBackendDevice> backendDevice = CoreBackendManager::self()->backend()->openDevice(device());

        if (backendDevice != nullptr) {
            Q_ASSERT(device().partitionTable());

            rval = backendDevice->createPartitionTable(*report, *device().partitionTable());
        } else
            report->line() << xi18nc("@info:progress", "Creating partition table failed: Could not open device <filename>%1</filename>.", device().deviceNode());
    } else if (device().type() == Device::Type::LVM_Device) {
        //TODO: figure what to do with LVM partitionTable
    }

    jobFinished(*report, rval);

    return rval;
}

QString CreatePartitionTableJob::description() const
{
    return xi18nc("@info:progress", "Create new partition table on device <filename>%1</filename>", device().deviceNode());
}
