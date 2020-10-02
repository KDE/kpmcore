/*
    SPDX-FileCopyrightText: 2008-2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2016-2018 Andrius Štikonas <andrius@stikonas.eu>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "jobs/checkfilesystemjob.h"
#include "core/partition.h"
#include "fs/filesystem.h"
#include "util/report.h"

#include <QDebug>

#include <KLocalizedString>

/** Creates a new CheckFileSystemJob
    @param p the Partition whose FileSystem is to be checked
*/
CheckFileSystemJob::CheckFileSystemJob(Partition& p) :
    Job(),
    m_Partition(p)
{
}

bool CheckFileSystemJob::run(Report& parent)
{
    Report* report = jobStarted(parent);

    // if we cannot check, assume everything is fine
    bool rval = true;

    if (partition().fileSystem().supportCheck() == FileSystem::cmdSupportFileSystem)
        rval = partition().fileSystem().check(*report, partition().deviceNode());

    jobFinished(*report, rval);

    return rval;
}

QString CheckFileSystemJob::description() const
{
    return xi18nc("@info:progress", "Check file system on partition <filename>%1</filename>", partition().deviceNode());
}
