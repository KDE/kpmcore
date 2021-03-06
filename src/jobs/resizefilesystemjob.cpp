/*
    SPDX-FileCopyrightText: 2008-2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2009 Andrew Coles <andrew.i.coles@googlemail.com>
    SPDX-FileCopyrightText: 2014-2018 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2016 Chantara Tith <tith.chantara@gmail.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "jobs/resizefilesystemjob.h"

#include "core/partition.h"
#include "core/device.h"

#include "backend/corebackend.h"
#include "backend/corebackendmanager.h"
#include "backend/corebackenddevice.h"
#include "backend/corebackendpartitiontable.h"

#include "fs/filesystem.h"

#include "util/report.h"
#include "util/capacity.h"

#include <memory>

#include <QDebug>

#include <KLocalizedString>

/** Creates a new ResizeFileSystemJob
    @param d the Device the FileSystem to be resized is on
    @param p the Partition the FileSystem to be resized is on
    @param newlength the new length for the FileSystem; if -1, the FileSystem will be resized to fill the entire Partition
*/
ResizeFileSystemJob::ResizeFileSystemJob(Device& d, Partition& p, qint64 newlength) :
    Job(),
    m_Device(d),
    m_Partition(p),
    m_Maximize(newlength == -1),
    m_NewLength(isMaximizing() ? partition().length() : newlength)
{
}

qint32 ResizeFileSystemJob::numSteps() const
{
    return 100;
}

bool ResizeFileSystemJob::run(Report& parent)
{
    Q_ASSERT(partition().fileSystem().firstSector() != -1);
    Q_ASSERT(partition().fileSystem().lastSector() != -1);
    Q_ASSERT(newLength() <= partition().length());

    if (partition().fileSystem().firstSector() == -1 || partition().fileSystem().lastSector() == -1 || newLength() > partition().length()) {
        qWarning() << "file system first sector: " << partition().fileSystem().firstSector() << ", last sector: " << partition().fileSystem().lastSector() << ", new length: " << newLength() << ", partition length: " << partition().length();
        return false;
    }

    bool rval = false;

    Report* report = jobStarted(parent);

    if (partition().fileSystem().length() == newLength()) {
        report->line() << xi18ncp("@info:progress", "The file system on partition <filename>%2</filename> already has the requested length of 1 sector.", "The file system on partition <filename>%2</filename> already has the requested length of %1 sectors.", newLength(), partition().deviceNode());
        rval = true;
    } else {
        report->line() << i18nc("@info:progress", "Resizing file system from %1 to %2 sectors.", partition().fileSystem().length(), newLength());

        FileSystem::CommandSupportType support = (newLength() < partition().fileSystem().length()) ? partition().fileSystem().supportShrink() : partition().fileSystem().supportGrow();

        switch (support) {
        case FileSystem::cmdSupportBackend: {
            Report* childReport = report->newChild();
            childReport->line() << xi18nc("@info:progress", "Resizing a %1 file system using internal backend functions.", partition().fileSystem().name());
            rval = resizeFileSystemBackend(*childReport);
            break;
        }

        case FileSystem::cmdSupportFileSystem: {
            const qint64 newLengthInByte = Capacity(newLength() * device().logicalSize()).toInt(Capacity::Unit::Byte);
            if (partition().isMounted())
                rval = partition().fileSystem().resizeOnline(*report, partition().deviceNode(), partition().mountPoint(), newLengthInByte);
            else
                rval = partition().fileSystem().resize(*report, partition().deviceNode(), newLengthInByte);
            break;
        }

        default:
            report->line() << xi18nc("@info:progress", "The file system on partition <filename>%1</filename> cannot be resized because there is no support for it.", partition().deviceNode());
            break;
        }

        if (rval)
            partition().fileSystem().setLastSector(partition().fileSystem().firstSector() + newLength() - 1);
    }

    jobFinished(*report, rval);

    return rval;
}

bool ResizeFileSystemJob::resizeFileSystemBackend(Report& report)
{
    bool rval = false;

    std::unique_ptr<CoreBackendDevice> backendDevice = CoreBackendManager::self()->backend()->openDevice(device());

    if (backendDevice) {
        std::unique_ptr<CoreBackendPartitionTable> backendPartitionTable = backendDevice->openPartitionTable();

        if (backendPartitionTable) {
            connect(CoreBackendManager::self()->backend(), &CoreBackend::progress, this, &ResizeFileSystemJob::progress);
            rval = backendPartitionTable->resizeFileSystem(report, partition(), newLength());
            disconnect(CoreBackendManager::self()->backend(), &CoreBackend::progress, this, &ResizeFileSystemJob::progress);

            if (rval) {
                report.line() << xi18nc("@info:progress", "Successfully resized file system using internal backend functions.");
                backendPartitionTable->commit();
            }
        } else
            report.line() << xi18nc("@info:progress", "Could not open partition <filename>%1</filename> while trying to resize the file system.", partition().deviceNode());

    } else
        report.line() << xi18nc("@info:progress", "Could not read geometry for partition <filename>%1</filename> while trying to resize the file system.", partition().deviceNode());

    return rval;
}

QString ResizeFileSystemJob::description() const
{
    if (isMaximizing())
        return xi18nc("@info:progress", "Maximize file system on <filename>%1</filename> to fill the partition", partition().deviceNode());

    return xi18ncp("@info:progress", "Resize file system on partition <filename>%2</filename> to 1 sector", "Resize file system on partition <filename>%2</filename> to %1 sectors", newLength(), partition().deviceNode());
}
