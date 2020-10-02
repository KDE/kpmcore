/*
    SPDX-FileCopyrightText: 2008-2012 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2014-2017 Andrius Štikonas <andrius@stikonas.eu>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "jobs/copyfilesystemjob.h"

#include "core/partition.h"
#include "core/device.h"
#include "core/copysourcedevice.h"
#include "core/copytargetdevice.h"

#include "fs/filesystem.h"

#include "util/report.h"

#include <KLocalizedString>

/** Creates a new CopyFileSystemJob
    @param targetdevice the Device the FileSystem is to be copied to
    @param targetpartition the Partition the FileSystem is to be copied to
    @param sourcedevice the Device the source FileSystem is on
    @param sourcepartition the Partition the source FileSystem is on
*/
CopyFileSystemJob::CopyFileSystemJob(Device& targetdevice, Partition& targetpartition, Device& sourcedevice, Partition& sourcepartition) :
    Job(),
    m_TargetDevice(targetdevice),
    m_TargetPartition(targetpartition),
    m_SourceDevice(sourcedevice),
    m_SourcePartition(sourcepartition)
{
}

qint32 CopyFileSystemJob::numSteps() const
{
    return 100;
}

bool CopyFileSystemJob::run(Report& parent)
{
    bool rval = false;

    Report* report = jobStarted(parent);

    if (targetPartition().fileSystem().length() < sourcePartition().fileSystem().length())
        report->line() << xi18nc("@info:progress", "Cannot copy file system: File system on target partition <filename>%1</filename> is smaller than the file system on source partition <filename>%2</filename>.", targetPartition().deviceNode(), sourcePartition().deviceNode());
    else if (sourcePartition().fileSystem().supportCopy() == FileSystem::cmdSupportFileSystem)
        rval = sourcePartition().fileSystem().copy(*report, targetPartition().deviceNode(), sourcePartition().deviceNode());
    else if (sourcePartition().fileSystem().supportCopy() == FileSystem::cmdSupportCore) {
        CopySourceDevice copySource(sourceDevice(), sourcePartition().fileSystem().firstByte(), sourcePartition().fileSystem().lastByte());
        CopyTargetDevice copyTarget(targetDevice(), targetPartition().fileSystem().firstByte(), targetPartition().fileSystem().lastByte());

        if (!copySource.open())
            report->line() << xi18nc("@info:progress", "Could not open file system on source partition <filename>%1</filename> for copying.", sourcePartition().deviceNode());
        else if (!copyTarget.open())
            report->line() << xi18nc("@info:progress", "Could not open file system on target partition <filename>%1</filename> for copying.", targetPartition().deviceNode());
        else {
            rval = copyBlocks(*report, copyTarget, copySource);
            report->line() << xi18nc("@info:progress", "Closing device. This may take a while, especially on slow devices like Memory Sticks.");
        }
    }

    if (rval) {
        // set the target file system to the length of the source
        const qint64 newLastSector = targetPartition().fileSystem().firstSector() + sourcePartition().fileSystem().length() - 1;

        targetPartition().fileSystem().setLastSector(newLastSector);

        // and set a new UUID, if the target filesystem supports UUIDs
        if (targetPartition().fileSystem().supportUpdateUUID() == FileSystem::cmdSupportFileSystem) {
            targetPartition().fileSystem().updateUUID(*report, targetPartition().deviceNode());
            targetPartition().fileSystem().setUUID(targetPartition().fileSystem().readUUID(targetPartition().deviceNode()));
        }
    }

    if (rval)
        rval = targetPartition().fileSystem().updateBootSector(*report, targetPartition().deviceNode());

    jobFinished(*report, rval);

    return rval;
}

QString CopyFileSystemJob::description() const
{
    return xi18nc("@info:progress", "Copy file system on partition <filename>%1</filename> to partition <filename>%2</filename>", sourcePartition().deviceNode(), targetPartition().deviceNode());
}
