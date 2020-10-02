/*
    SPDX-FileCopyrightText: 2008-2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2012-2019 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2015 Teo Mrnjavac <teo@kde.org>
    SPDX-FileCopyrightText: 2016 Chantara Tith <tith.chantara@gmail.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "ops/restoreoperation.h"

#include "core/partition.h"
#include "core/device.h"
#include "core/partitiontable.h"
#include "core/partitionnode.h"

#include "jobs/createpartitionjob.h"
#include "jobs/deletepartitionjob.h"
#include "jobs/checkfilesystemjob.h"
#include "jobs/restorefilesystemjob.h"
#include "jobs/resizefilesystemjob.h"

#include "fs/filesystem.h"
#include "fs/filesystemfactory.h"
#include "fs/luks.h"

#include "util/capacity.h"
#include "util/report.h"

#include <QDebug>
#include <QString>
#include <QFileInfo>

#include <KLocalizedString>

/** Creates a new RestoreOperation.
    @param d the Device to restore the Partition to
    @param p pointer to the Partition that will be restored. May not be nullptr.
    @param filename name of the image file to restore from
*/
RestoreOperation::RestoreOperation(Device& d, Partition* p, const QString& filename) :
    Operation(),
    m_TargetDevice(d),
    m_RestorePartition(p),
    m_FileName(filename),
    m_OverwrittenPartition(nullptr),
    m_MustDeleteOverwritten(false),
    m_ImageLength(QFileInfo(filename).size() / 512), // 512 being the "sector size" of an image file.
    m_CreatePartitionJob(nullptr),
    m_RestoreJob(nullptr),
    m_CheckTargetJob(nullptr),
    m_MaximizeJob(nullptr)
{
    restorePartition().setState(Partition::State::Restore);

    Q_ASSERT(targetDevice().partitionTable());

    Partition* dest = targetDevice().partitionTable()->findPartitionBySector(restorePartition().firstSector(), PartitionRole(PartitionRole::Primary | PartitionRole::Logical | PartitionRole::Unallocated));

    if (dest == nullptr)
        qWarning() << "destination partition not found at sector " << restorePartition().firstSector();

    Q_ASSERT(dest);

    if (dest && !dest->roles().has(PartitionRole::Unallocated)) {
        restorePartition().setLastSector(dest->lastSector());
        setOverwrittenPartition(dest);
        removePreviewPartition(targetDevice(), *dest);
    }

    if (!overwrittenPartition())
        addJob(m_CreatePartitionJob = new CreatePartitionJob(targetDevice(), restorePartition()));

    addJob(m_RestoreJob = new RestoreFileSystemJob(targetDevice(), restorePartition(), fileName()));
    addJob(m_CheckTargetJob = new CheckFileSystemJob(restorePartition()));
    addJob(m_MaximizeJob = new ResizeFileSystemJob(targetDevice(), restorePartition()));
}

RestoreOperation::~RestoreOperation()
{
    if (status() == StatusPending)
        delete m_RestorePartition;

    if (status() == StatusFinishedSuccess || status() == StatusFinishedWarning || status() == StatusError)
        cleanupOverwrittenPartition();
}

bool RestoreOperation::targets(const Device& d) const
{
    return d == targetDevice();
}

bool RestoreOperation::targets(const Partition& p) const
{
    return p == restorePartition();
}

void RestoreOperation::preview()
{
    insertPreviewPartition(targetDevice(), restorePartition());
}

void RestoreOperation::undo()
{
    removePreviewPartition(targetDevice(), restorePartition());

    if (overwrittenPartition())
        insertPreviewPartition(targetDevice(), *overwrittenPartition());
}

bool RestoreOperation::execute(Report& parent)
{
    bool rval = false;
    bool warning = false;

    Report* report = parent.newChild(description());

    if (overwrittenPartition())
        restorePartition().setPartitionPath(overwrittenPartition()->partitionPath());

    if (overwrittenPartition() || (rval = createPartitionJob()->run(*report))) {
        restorePartition().setState(Partition::State::None);

        if ((rval = restoreJob()->run(*report))) {
            if ((rval = checkTargetJob()->run(*report))) {
                // If the partition was written over an existing one, the partition itself may now
                // be larger than the filesystem, so maximize the filesystem to the partition's size
                // or the image length, whichever is larger. If this fails, don't return an error, just
                // warn the user.
                if ((warning = !maximizeJob()->run(*report)))
                    report->line() << xi18nc("@info:status", "<warning>Maximizing file system on target partition <filename>%1</filename> to the size of the partition failed.</warning>", restorePartition().deviceNode());
            } else
                report->line() << xi18nc("@info:status", "Checking target file system on partition <filename>%1</filename> after the restore failed.", restorePartition().deviceNode());
        } else {
            if (!overwrittenPartition())
                DeletePartitionJob(targetDevice(), restorePartition()).run(*report);

            report->line() << xi18nc("@info:status", "Restoring file system failed.");
        }
    } else
        report->line() << xi18nc("@info:status", "Creating the destination partition to restore to failed.");

    if (rval)
        setStatus(warning ? StatusFinishedWarning : StatusFinishedSuccess);
    else
        setStatus(StatusError);

    report->setStatus(xi18nc("@info:status (success, error, warning...) of operation", "%1: %2", description(), statusText()));

    return rval;
}

QString RestoreOperation::description() const
{
    if (overwrittenPartition())
        return xi18nc("@info:status", "Restore partition from <filename>%1</filename> to <filename>%2</filename>", fileName(), overwrittenPartition()->deviceNode());

    return xi18nc("@info:status", "Restore partition on <filename>%1</filename> at %2 from <filename>%3</filename>", targetDevice().deviceNode(), Capacity::formatByteSize(restorePartition().firstSector() * targetDevice().logicalSize()), fileName());
}

void RestoreOperation::setOverwrittenPartition(Partition* p)
{
    // This is copied from CopyOperation. One day we might create a common base class ;-)

    cleanupOverwrittenPartition();
    m_OverwrittenPartition = p;
    m_MustDeleteOverwritten = (p && p->state() == Partition::State::None);
}

void RestoreOperation::cleanupOverwrittenPartition()
{
    if (mustDeleteOverwritten()) {
        delete overwrittenPartition();
        m_OverwrittenPartition = nullptr;
    }
}

/** Can a Partition be restored to somewhere?
    @param p the Partition in question, may be nullptr.
    @return true if a Partition can be restored to @p p.
 */
bool RestoreOperation::canRestore(const Partition* p)
{
    if (p == nullptr)
        return false;

    if (p->isMounted())
        return false;

    if (p->roles().has(PartitionRole::Extended))
        return false;

    if (p->roles().has(PartitionRole::Luks)) {
        const FS::luks* luksFs = static_cast<const FS::luks*>(&p->fileSystem());
        return luksFs->mapperName().isEmpty();
    }

    return true;
}
/** Creates a new Partition to restore to.
    @param device the Device to create the Partition on
    @param parent the parent PartitionNode
    @param start start sector of the Partition
    @param filename name of the image file to restore from
*/
Partition* RestoreOperation::createRestorePartition(const Device& device, PartitionNode& parent, qint64 start, const QString& filename)
{
    PartitionRole::Roles r = PartitionRole::Primary;

    if (!parent.isRoot())
        r = PartitionRole::Logical;

    QFileInfo fileInfo(filename);

    if (!fileInfo.exists())
        return nullptr;

    const qint64 end = start + fileInfo.size() / device.logicalSize() - 1;
    Partition* p = new Partition(&parent, device, PartitionRole(r), FileSystemFactory::create(FileSystem::Type::Unknown, start, end, device.logicalSize()), start, end, QString());

    p->setState(Partition::State::Restore);
    return p;
}

