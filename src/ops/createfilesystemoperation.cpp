/*
    SPDX-FileCopyrightText: 2008-2011 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2014-2016 Andrius Štikonas <andrius@stikonas.eu>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "ops/createfilesystemoperation.h"

#include "core/partition.h"
#include "core/device.h"

#include "jobs/deletefilesystemjob.h"
#include "jobs/createfilesystemjob.h"
#include "jobs/checkfilesystemjob.h"

#include "fs/filesystem.h"
#include "fs/filesystemfactory.h"

#include <QString>

#include <KLocalizedString>

/** Creates a new CreateFileSystemOperation.
    @param d the Device to create the new FileSystem on
    @param p the Partition to create the new FileSystem in
    @param newType the type of the new FileSystem
*/
CreateFileSystemOperation::CreateFileSystemOperation(Device& d, Partition& p, FileSystem::Type newType) :
    Operation(),
    m_TargetDevice(d),
    m_Partition(p),
    m_NewFileSystem(FileSystemFactory::cloneWithNewType(newType, partition().fileSystem())),
    m_OldFileSystem(&p.fileSystem()),
    m_DeleteJob(new DeleteFileSystemJob(targetDevice(), partition())),
    m_CreateJob(new CreateFileSystemJob(targetDevice(), partition())),
    m_CheckJob(new CheckFileSystemJob(partition()))
{
    // We never know anything about the number of used sectors on a new file system.
    newFileSystem()->setSectorsUsed(-1);

    addJob(deleteJob());
    addJob(createJob());
    addJob(checkJob());
}

CreateFileSystemOperation::~CreateFileSystemOperation()
{
    if (&partition().fileSystem() == newFileSystem())
        delete oldFileSystem();
    else
        delete newFileSystem();
}

bool CreateFileSystemOperation::targets(const Device& d) const
{
    return d == targetDevice();
}

bool CreateFileSystemOperation::targets(const Partition& p) const
{
    return p == partition();
}

void CreateFileSystemOperation::preview()
{
    partition().setFileSystem(newFileSystem());
}

void CreateFileSystemOperation::undo()
{
    partition().setFileSystem(oldFileSystem());
}

bool CreateFileSystemOperation::execute(Report& parent)
{
    preview();

    return Operation::execute(parent);
}

QString CreateFileSystemOperation::description() const
{
    return xi18nc("@info:status", "Create filesystem %1 on partition <filename>%2</filename>", newFileSystem()->name(), partition().deviceNode());
}
