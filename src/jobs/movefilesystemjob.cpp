/*
    SPDX-FileCopyrightText: 2008-2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2014-2017 Andrius Štikonas <andrius@stikonas.eu>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "jobs/movefilesystemjob.h"

#include "core/partition.h"
#include "core/device.h"
#include "core/copysourcedevice.h"
#include "core/copytargetdevice.h"

#include "util/report.h"

#include <KLocalizedString>

/** Creates a new MoveFileSystemJob
    @param d the Device the Partition to move is on
    @param p the Partition to move
    @param newstart the new start sector for the Partition
*/
MoveFileSystemJob::MoveFileSystemJob(Device& d, Partition& p, qint64 newstart) :
    Job(),
    m_Device(d),
    m_Partition(p),
    m_NewStart(newstart)
{
}

qint32 MoveFileSystemJob::numSteps() const
{
    return 100;
}

bool MoveFileSystemJob::run(Report& parent)
{
    bool rval = false;

    Report* report = jobStarted(parent);

    // A scope for moveSource and moveTarget, so CopyTargetDevice's dtor runs before we
    // say we're finished: The CopyTargetDevice dtor asks the backend to close the device
    // and that may take a while.
    {
        qint64 length = partition().fileSystem().lastByte() - partition().fileSystem().firstByte();
        CopySourceDevice moveSource(device(), partition().fileSystem().firstByte(), partition().fileSystem().lastByte());
        CopyTargetDevice moveTarget(device(), newStart() * device().logicalSize(), newStart() * device().logicalSize() + length);

        if (!moveSource.open())
            report->line() << xi18nc("@info:progress", "Could not open file system on partition <filename>%1</filename> for moving.", partition().deviceNode());
        else if (!moveTarget.open())
            report->line() << xi18nc("@info:progress", "Could not create target for moving file system on partition <filename>%1</filename>.", partition().deviceNode());
        else {
            rval = copyBlocks(*report, moveTarget, moveSource);

            if (rval) {
                const qint64 savedLength = partition().fileSystem().length() - 1;
                partition().fileSystem().setFirstSector(newStart());
                partition().fileSystem().setLastSector(newStart() + savedLength);
            } else if (!rollbackCopyBlocks(*report, moveTarget, moveSource))
                report->line() << xi18nc("@info:progress", "Rollback for file system on partition <filename>%1</filename> failed.", partition().deviceNode());

            report->line() << xi18nc("@info:progress", "Closing device. This may take a few seconds.");
        }
    }

    if (rval)
        rval = partition().fileSystem().updateBootSector(*report, partition().deviceNode());

    jobFinished(*report, rval);

    return rval;
}

QString MoveFileSystemJob::description() const
{
    return xi18nc("@info:progress", "Move the file system on partition <filename>%1</filename> to sector %2", partition().deviceNode(), newStart());
}
