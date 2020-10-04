/*
    SPDX-FileCopyrightText: 2008-2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2013-2020 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2016 Chantara Tith <tith.chantara@gmail.com>
    SPDX-FileCopyrightText: 2018 Caio Jordão Carvalho <caiojcarvalho@gmail.com>
    SPDX-FileCopyrightText: 2019 Yuri Chornoivan <yurchor@ukr.net>
    SPDX-FileCopyrightText: 2020 Gaël PORTAY <gael.portay@collabora.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "jobs/createpartitionjob.h"

#include "backend/corebackend.h"
#include "backend/corebackendmanager.h"
#include "backend/corebackenddevice.h"
#include "backend/corebackendpartitiontable.h"

#include "core/partition.h"
#include "core/device.h"
#include "core/lvmdevice.h"

#include "util/report.h"

#include <KLocalizedString>

/** Creates a new CreatePartitionJob
    @param d the Device the Partition to be created will be on
    @param p the Partition to create
*/
CreatePartitionJob::CreatePartitionJob(Device& d, Partition& p) :
    Job(),
    m_Device(d),
    m_Partition(p)
{
}

bool CreatePartitionJob::run(Report& parent)
{
    Q_ASSERT(partition().devicePath() == device().deviceNode());

    bool rval = false;
    Report* report = jobStarted(parent);

    if (device().partitionTable()->type() == PartitionTable::TableType::none) {
        partition().setPartitionPath(device().deviceNode());
        partition().setState(Partition::State::None);
        rval = true;
        jobFinished(*report, rval);
        return rval;
    }

    if (device().type() == Device::Type::Disk_Device || device().type() == Device::Type::SoftwareRAID_Device) {
        std::unique_ptr<CoreBackendDevice> backendDevice = CoreBackendManager::self()->backend()->openDevice(device());

        if (backendDevice) {
            std::unique_ptr<CoreBackendPartitionTable> backendPartitionTable = backendDevice->openPartitionTable();

            if (backendPartitionTable) {
                QString partitionPath = backendPartitionTable->createPartition(*report, partition());

                if (!partitionPath.isEmpty()) {
                    rval = true;
                    partition().setPartitionPath(partitionPath);
                    partition().setState(Partition::State::None);
                    backendPartitionTable->commit();
                    // The UUID is supported by GPT only; it is generated automatically once the creation of a partition.
                    // Store the generated UUID to the partition object if no UUID is set.
                    if (m_Device.partitionTable()->type() == PartitionTable::gpt && partition().uuid().isEmpty())
                        partition().setUUID(backendPartitionTable->getPartitionUUID(*report, partition()));
                } else
                    report->line() << xi18nc("@info/plain", "Failed to add partition <filename>%1</filename> to device <filename>%2</filename>.", partition().deviceNode(), device().deviceNode());
            } else
                report->line() << xi18nc("@info:progress", "Could not open partition table on device <filename>%1</filename> to create new partition <filename>%2</filename>.", device().deviceNode(), partition().deviceNode());
        } else
            report->line() << xi18nc("@info:progress", "Could not open device <filename>%1</filename> to create new partition <filename>%2</filename>.", device().deviceNode(), partition().deviceNode());
    } else if (device().type() == Device::Type::LVM_Device) {
        LvmDevice& dev = dynamic_cast<LvmDevice&>(device());
        partition().setState(Partition::State::None);

        QString partPath = partition().partitionPath();
        QString lvname   = partPath.right(partPath.length() - partPath.lastIndexOf(QStringLiteral("/")) - 1);
        rval = LvmDevice::createLV(*report, dev, partition(), lvname);
    }

    jobFinished(*report, rval);

    return rval;
}

QString CreatePartitionJob::description() const
{
    if (partition().number() > 0)
        return xi18nc("@info:progress", "Create new partition <filename>%1</filename>", partition().deviceNode());

    return xi18nc("@info:progress", "Create new partition on device <filename>%1</filename>", device().deviceNode());
}
