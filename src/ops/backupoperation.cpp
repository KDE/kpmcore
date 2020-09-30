/*
    SPDX-FileCopyrightText: 2008-2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2012-2018 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2016 Teo Mrnjavac <teo@kde.org>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "ops/backupoperation.h"

#include "core/partition.h"
#include "core/device.h"

#include "jobs/backupfilesystemjob.h"

#include "util/capacity.h"

#include <QString>

#include <KLocalizedString>

/** Creates a new BackupOperation.
    @param d the Device where the FileSystem to back up is on
    @param p the Partition where the FileSystem to back up is in
    @param filename the name of the file to back up to
*/
BackupOperation::BackupOperation(Device& d, Partition& p, const QString& filename) :
    Operation(),
    m_TargetDevice(d),
    m_BackupPartition(p),
    m_FileName(filename),
    m_BackupJob(new BackupFileSystemJob(targetDevice(), backupPartition(), fileName()))
{
    addJob(backupJob());
}

QString BackupOperation::description() const
{
    return xi18nc("@info:status", "Backup partition <filename>%1</filename> (%2, %3) to <filename>%4</filename>", backupPartition().deviceNode(), Capacity::formatByteSize(backupPartition().capacity()), backupPartition().fileSystem().name(), fileName());
}

/** Can the given Partition be backed up?
    @param p The Partition in question, may be nullptr.
    @return true if @p p can be backed up.
*/
bool BackupOperation::canBackup(const Partition* p)
{
    if (p == nullptr)
        return false;

    if (p->isMounted())
        return false;

    if (p->state() == Partition::State::New || p->state() == Partition::State::Copy || p->state() == Partition::State::Restore)
        return false;

    return p->fileSystem().supportBackup() != FileSystem::cmdSupportNone;
}

