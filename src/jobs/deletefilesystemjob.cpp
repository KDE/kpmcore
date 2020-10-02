/*
    SPDX-FileCopyrightText: 2008-2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2012-2018 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2016 Chantara Tith <tith.chantara@gmail.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "jobs/deletefilesystemjob.h"

#include "backend/corebackend.h"
#include "backend/corebackendmanager.h"
#include "backend/corebackenddevice.h"
#include "backend/corebackendpartitiontable.h"

#include "core/partition.h"
#include "core/device.h"

#include "util/helpers.h"
#include "util/report.h"

#include <QDebug>

#include <KLocalizedString>

/** Creates a new DeleteFileSystemJob
    @param d the Device the FileSystem to delete is on
    @param p the Partition the FileSystem to delete is on
*/
DeleteFileSystemJob::DeleteFileSystemJob(Device& d, Partition& p) :
    Job(),
    m_Device(d),
    m_Partition(p)
{
}

bool DeleteFileSystemJob::run(Report& parent)
{
    Q_ASSERT(device().deviceNode() == partition().devicePath());

    if (device().deviceNode() != partition().devicePath()) {
        qWarning() << "deviceNode: " << device().deviceNode() << ", partition path: " << partition().devicePath();
        return false;
    }

    bool rval = false;

    Report* report = jobStarted(parent);

    if (isMounted(partition().partitionPath())) {
        report->line() << xi18nc("@info:progress", "Could not delete file system: file system on <filename>%1</filename> is mounted.", partition().deviceNode());
        jobFinished(*report, rval);
        return false;
    }

    if (partition().roles().has(PartitionRole::Extended)) {
        rval = true;
    } else if (device().type() == Device::Type::LVM_Device) {
        rval = true;
    }
    else {
        if (!partition().fileSystem().remove(*report, partition().deviceNode())) {
            jobFinished(*report, rval);
            return false;
        }

        std::unique_ptr<CoreBackendDevice> backendDevice = CoreBackendManager::self()->backend()->openDevice(device());

        if (backendDevice) {
            std::unique_ptr<CoreBackendPartitionTable> backendPartitionTable = backendDevice->openPartitionTable();

            if (backendPartitionTable) {
                rval = backendPartitionTable->clobberFileSystem(*report, partition());

                if (!rval)
                    report->line() << xi18nc("@info:progress", "Could not delete file system on <filename>%1</filename>.", partition().deviceNode());
                else
                    backendPartitionTable->commit();
            } else
                report->line() << xi18nc("@info:progress", "Could not open partition table on device <filename>%1</filename> to delete file system on <filename>%2</filename>.", device().deviceNode(), partition().deviceNode());

        } else
            report->line() << xi18nc("@info:progress", "Could not delete file system signature for partition <filename>%1</filename>: Failed to open device <filename>%2</filename>.", partition().deviceNode(), device().deviceNode());
    }

    jobFinished(*report, rval);

    return rval;
}

QString DeleteFileSystemJob::description() const
{
    return xi18nc("@info:progress", "Delete file system on <filename>%1</filename>", partition().deviceNode());
}
