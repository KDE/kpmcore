/*
    SPDX-FileCopyrightText: 2008-2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2014-2018 Andrius Štikonas <andrius@stikonas.eu>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "jobs/restorefilesystemjob.h"

#include "backend/corebackend.h"
#include "backend/corebackendmanager.h"
#include "backend/corebackenddevice.h"
#include "backend/corebackendpartitiontable.h"

#include "core/partition.h"
#include "core/device.h"
#include "core/copysourcefile.h"
#include "core/copytargetdevice.h"

#include "fs/filesystem.h"
#include "fs/filesystemfactory.h"

#include "util/report.h"

#include <KLocalizedString>

/** Creates a new RestoreFileSystemJob
    @param targetdevice the Device the FileSystem is to be restored to
    @param targetpartition the Partition the FileSystem is to be restore to
    @param filename the file name with the image file to restore
*/
RestoreFileSystemJob::RestoreFileSystemJob(Device& targetdevice, Partition& targetpartition, const QString& filename) :
    Job(),
    m_TargetDevice(targetdevice),
    m_TargetPartition(targetpartition),
    m_FileName(filename)
{
}

qint32 RestoreFileSystemJob::numSteps() const
{
    return 100;
}

bool RestoreFileSystemJob::run(Report& parent)
{
    // Restoring is file system independent because we currently have no way of
    // detecting the file system in a given image file. We cannot even find out if the
    // file the user gave us is a valid image file or just some junk.

    bool rval = false;

    Report* report = jobStarted(parent);

    // Again, a scope for copyTarget and copySource. See MoveFileSystemJob::run()
    {
        // FileSystems are restored to _partitions_, so don't use first and last sector of file system here
        CopyTargetDevice copyTarget(targetDevice(), targetPartition().firstByte(), targetPartition().lastByte());
        CopySourceFile copySource(fileName());

        if (!copySource.open())
            report->line() << xi18nc("@info:progress", "Could not open backup file <filename>%1</filename> to restore from.", fileName());
        else if (!copyTarget.open())
            report->line() << xi18nc("@info:progress", "Could not open target partition <filename>%1</filename> to restore to.", targetPartition().deviceNode());
        else {
            rval = copyBlocks(*report, copyTarget, copySource);

            if (rval) {
                // create a new file system for what was restored with the length of the image file
                const qint64 newLastSector = targetPartition().firstSector() + copySource.length() - 1;

                std::unique_ptr<CoreBackendDevice> backendDevice = CoreBackendManager::self()->backend()->openDevice(targetDevice());

                FileSystem::Type t = FileSystem::Type::Unknown;

                if (backendDevice) {
                    std::unique_ptr<CoreBackendPartitionTable> backendPartitionTable = backendDevice->openPartitionTable();

                    if (backendPartitionTable)
                        t = backendPartitionTable->detectFileSystemBySector(*report, targetDevice(), targetPartition().firstSector());
                }

                FileSystem* fs = FileSystemFactory::create(t, targetPartition().firstSector(), newLastSector, targetPartition().sectorSize());

                targetPartition().deleteFileSystem();
                targetPartition().setFileSystem(fs);
            }

            report->line() << xi18nc("@info:progress", "Closing device. This may take a few seconds.");
        }
    }

    jobFinished(*report, rval);

    return rval;
}

QString RestoreFileSystemJob::description() const
{
    return xi18nc("@info:progress", "Restore the file system from file <filename>%1</filename> to partition <filename>%2</filename>", fileName(), targetPartition().deviceNode());
}
