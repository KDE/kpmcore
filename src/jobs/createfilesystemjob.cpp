/*
    SPDX-FileCopyrightText: 2008-2011 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2014-2018 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2016 Chantara Tith <tith.chantara@gmail.com>
    SPDX-FileCopyrightText: 2018 Caio Jordão Carvalho <caiojcarvalho@gmail.com>
    SPDX-FileCopyrightText: 2020 Arnaud Ferraris <arnaud.ferraris@collabora.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "jobs/createfilesystemjob.h"

#include "backend/corebackend.h"
#include "backend/corebackendmanager.h"
#include "backend/corebackenddevice.h"
#include "backend/corebackendpartitiontable.h"

#include "core/device.h"
#include "core/partition.h"

#include "fs/filesystem.h"

#include "util/report.h"

#include <KLocalizedString>

/** Creates a new CreateFileSystemJob
    @param p the Partition the FileSystem to create is on
*/
CreateFileSystemJob::CreateFileSystemJob(Device& d, Partition& p, const QString& label) :
    Job(),
    m_Device(d),
    m_Partition(p),
    m_Label(label)
{
}

bool CreateFileSystemJob::run(Report& parent)
{
    bool rval = false;

    Report* report = jobStarted(parent);

    if (partition().fileSystem().type() == FileSystem::Type::Unformatted)
        return true;

    bool createResult;
    if (partition().fileSystem().supportCreate() == FileSystem::cmdSupportFileSystem) {
        if (partition().fileSystem().supportCreateWithLabel() == FileSystem::cmdSupportFileSystem) {
            createResult = partition().fileSystem().createWithLabel(*report, partition().deviceNode(), m_Label);
        } else {
            createResult = partition().fileSystem().create(*report, partition().deviceNode());
        }
        if (createResult) {
            if (device().type() == Device::Type::Disk_Device || device().type() == Device::Type::SoftwareRAID_Device) {
                std::unique_ptr<CoreBackendDevice> backendDevice = CoreBackendManager::self()->backend()->openDevice(device());

                if (backendDevice) {
                    std::unique_ptr<CoreBackendPartitionTable> backendPartitionTable = backendDevice->openPartitionTable();

                    if (backendPartitionTable) {
                        if (backendPartitionTable->setPartitionSystemType(*report, partition())) {
                            rval = true;
                            backendPartitionTable->commit();
                        } else
                            report->line() << xi18nc("@info:progress", "Failed to set the system type for the file system on partition <filename>%1</filename>.", partition().deviceNode());
                    } else
                        report->line() << xi18nc("@info:progress", "Could not open partition table on device <filename>%1</filename> to set the system type for partition <filename>%2</filename>.", device().deviceNode(), partition().deviceNode());
                } else
                    report->line() << xi18nc("@info:progress", "Could not open device <filename>%1</filename> to set the system type for partition <filename>%2</filename>.", device().deviceNode(), partition().deviceNode());
            } else if (device().type() == Device::Type::LVM_Device) {
                rval = true;
            }
        }
    }

    jobFinished(*report, rval);

    return rval;
}

QString CreateFileSystemJob::description() const
{
    return xi18nc("@info:progress", "Create file system <filename>%1</filename> on partition <filename>%2</filename>", partition().fileSystem().name(), partition().deviceNode());
}
