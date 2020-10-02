/*
    SPDX-FileCopyrightText: 2008-2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2014-2018 Andrius Štikonas <andrius@stikonas.eu>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "jobs/setfilesystemlabeljob.h"

#include "backend/corebackend.h"
#include "backend/corebackendmanager.h"
#include "backend/corebackenddevice.h"
#include "backend/corebackendpartitiontable.h"

#include "core/device_p.h"
#include "core/operationstack.h"
#include "core/partition.h"

#include "fs/filesystem.h"

#include "util/report.h"

#include <KLocalizedString>

#include <memory>

/** Creates a new SetFileSystemLabelJob
    @param p the Partition the FileSystem whose label is to be set is on
    @param newlabel the new label
*/
SetFileSystemLabelJob::SetFileSystemLabelJob(Partition& p, const QString& newlabel) :
    Job(),
    m_Partition(p),
    m_Label(newlabel)
{
}

bool SetFileSystemLabelJob::run(Report& parent)
{
    bool rval = true;

    Report* report = jobStarted(parent);

    // If there's no support for file system label setting for this file system,
    // just ignore the request and say all is well. This helps in operations because
    // we don't have to check for support to avoid having a failed job.
    if (partition().fileSystem().supportSetLabel() == FileSystem::cmdSupportNone)
        report->line() << xi18nc("@info:progress", "File system on partition <filename>%1</filename> does not support setting labels. Job ignored.", partition().deviceNode());
    else if (partition().fileSystem().supportSetLabel() == FileSystem::cmdSupportFileSystem && !partition().isMounted()) {
        rval = partition().fileSystem().writeLabel(*report, partition().deviceNode(), label());

        if (rval)
            partition().fileSystem().setLabel(label());
    }
    else if (partition().fileSystem().supportSetLabelOnline() == FileSystem::cmdSupportFileSystem && partition().isMounted()) {
        rval = partition().fileSystem().writeLabelOnline(*report, partition().deviceNode(), partition().mountPoint(), label());

        if (rval)
            partition().fileSystem().setLabel(label());
    }
    else
        rval = false;

    // A hack to reread partition table (commit() should be called even on non DiskDevices)
    Device dev(std::make_shared<DevicePrivate>(), QString(), QString(), 0, 0, QString(), Device::Type::Unknown_Device);
    std::unique_ptr<CoreBackendDevice> backendDevice = CoreBackendManager::self()->backend()->openDevice(dev);
    if (backendDevice) {
        std::unique_ptr<CoreBackendPartitionTable> backendPartitionTable = backendDevice->openPartitionTable();

        if (backendPartitionTable)
            backendPartitionTable->commit();
    }

    jobFinished(*report, rval);

    return rval;
}

QString SetFileSystemLabelJob::description() const
{
    return xi18nc("@info:progress", "Set the file system label on partition <filename>%1</filename> to \"%2\"", partition().deviceNode(), label());
}
