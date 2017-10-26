/*************************************************************************
 *  Copyright (C) 2010 by Volker Lanz <vl@fidra.de>                      *
 *  Copyright (C) 2016 by Andrius Štikonas <andrius@stikonas.eu>         *
 *                                                                       *
 *  This program is free software; you can redistribute it and/or        *
 *  modify it under the terms of the GNU General Public License as       *
 *  published by the Free Software Foundation; either version 3 of       *
 *  the License, or (at your option) any later version.                  *
 *                                                                       *
 *  This program is distributed in the hope that it will be useful,      *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of       *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        *
 *  GNU General Public License for more details.                         *
 *                                                                       *
 *  You should have received a copy of the GNU General Public License    *
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.*
 *************************************************************************/

#include "jobs/shredfilesystemjob.h"

#include "core/partition.h"
#include "core/device.h"
#include "core/copysourceshred.h"
#include "core/copytargetdevice.h"

#include "fs/filesystem.h"
#include "fs/filesystemfactory.h"

#include "util/report.h"

#include <QDebug>

#include <KLocalizedString>

/** Creates a new ShredFileSystemJob
    @param d the Device the FileSystem is on
    @param p the Partition the FileSystem is in
*/
ShredFileSystemJob::ShredFileSystemJob(Device& d, Partition& p, bool randomShred) :
    Job(),
    m_Device(d),
    m_Partition(p),
    m_RandomShred(randomShred)
{
}

qint32 ShredFileSystemJob::numSteps() const
{
    return 100;
}

bool ShredFileSystemJob::run(Report& parent)
{
    Q_ASSERT(device().deviceNode() == partition().devicePath());

    if (device().deviceNode() != partition().devicePath()) {
        qWarning() << "deviceNode: " << device().deviceNode() << ", partition path: " << partition().devicePath();
        return false;
    }

    bool rval = false;

    Report* report = jobStarted(parent);

    // Again, a scope for copyTarget and copySource. See MoveFileSystemJob::run()
    {
        CopyTargetDevice copyTarget(device(), partition().fileSystem().firstByte(), partition().fileSystem().lastByte());
        CopySourceShred copySource(partition().capacity(), m_RandomShred);

        if (!copySource.open())
            report->line() << xi18nc("@info:progress", "Could not open random data source to overwrite file system.");
        else if (!copyTarget.open())
            report->line() << xi18nc("@info:progress", "Could not open target partition <filename>%1</filename> to restore to.", partition().deviceNode());
        else {
            rval = copyBlocks(*report, copyTarget, copySource);
            report->line() << i18nc("@info:progress", "Closing device. This may take a few seconds.");
        }
    }

    jobFinished(*report, rval);

    return rval;
}

QString ShredFileSystemJob::description() const
{
    return xi18nc("@info:progress", "Shred the file system on <filename>%1</filename>", partition().deviceNode());
}
