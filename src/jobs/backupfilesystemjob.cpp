/*
    SPDX-FileCopyrightText: 2008-2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2014-2018 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2016 Chantara Tith <tith.chantara@gmail.com>
    SPDX-FileCopyrightText: 2018 Huzaifa Faruqui <huzaifafaruqui@gmail.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "jobs/backupfilesystemjob.h"

#include "core/partition.h"
#include "core/device.h"
#include "core/copysourcedevice.h"
#include "core/copytargetfile.h"

#include "fs/filesystem.h"
#include "util/externalcommand.h"
#include "util/report.h"

#include <KLocalizedString>

/** Creates a new BackupFileSystemJob
    @param sourcedevice the device the FileSystem to back up is on
    @param sourcepartition the Partition the FileSystem to back up is on
    @param filename name of the file to backup to
*/
BackupFileSystemJob::BackupFileSystemJob(Device& sourcedevice, Partition& sourcepartition, const QString& filename) :
    Job(),
    m_SourceDevice(sourcedevice),
    m_SourcePartition(sourcepartition),
    m_FileName(filename)
{
}

qint32 BackupFileSystemJob::numSteps() const
{
    return 100;
}

bool BackupFileSystemJob::run(Report& parent)
{
    bool rval = false;

    Report* report = jobStarted(parent);

    if (sourcePartition().fileSystem().supportBackup() == FileSystem::cmdSupportFileSystem)
        rval = sourcePartition().fileSystem().backup(*report, sourceDevice(), sourcePartition().deviceNode(), fileName());
    else if (sourcePartition().fileSystem().supportBackup() == FileSystem::cmdSupportCore) {
        CopySourceDevice copySource(sourceDevice(), sourcePartition().fileSystem().firstByte(), sourcePartition().fileSystem().lastByte());
        CopyTargetFile copyTarget(fileName());

        if (!copySource.open())
            report->line() << xi18nc("@info:progress", "Could not open file system on source partition <filename>%1</filename> for backup.", sourcePartition().deviceNode());
        else if (!copyTarget.open())
            report->line() << xi18nc("@info:progress", "Could not create backup file <filename>%1</filename>.", fileName());
        else
            rval = copyBlocks(*report, copyTarget, copySource);
    }

    jobFinished(*report, rval);

    return rval;
}

QString BackupFileSystemJob::description() const
{
    return xi18nc("@info:progress", "Back up file system on partition <filename>%1</filename> to <filename>%2</filename>", sourcePartition().deviceNode(), fileName());
}
