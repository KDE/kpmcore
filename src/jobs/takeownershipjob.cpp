/*
    SPDX-FileCopyrightText: 2026 Ramil Nurmanov <ramil2004nur@gmail.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "jobs/takeownershipjob.h"

#include "core/partition.h"

#include "ops/takeownershipoperation.h"
#include "util/externalcommand.h"
#include "util/report.h"

#include <KLocalizedString>

TakeOwnershipJob::TakeOwnershipJob(Partition &p, const QString &userName, bool recursive)
    : Job(), m_Partition(p), m_UserName(userName), m_Recursive(recursive)
{
}

bool TakeOwnershipJob::run(Report &parent)
{
    Report *report = jobStarted(parent);

    if (!TakeOwnershipOperation::supportsOwnership(partition().fileSystem().type()))
    {
        report->line() << xi18nc("@info:status", "File system on <filename>%1</filename> does not support ownership.",
                                 partition().mountPoint());
        jobFinished(*report, false);
        return false;
    }

    if (TakeOwnershipOperation::isCriticalMountPoint(partition().mountPoint()))
    {
        report->line() << xi18nc("@info:status", "Refusing to change ownership of critical mount point <filename>%1</filename>.",
                                 partition().mountPoint());
        jobFinished(*report, false);
        return false;
    }

    QStringList args;
    if (m_Recursive)
        args << QStringLiteral("--recursive");

    const QString ownerSpec = m_UserName + QLatin1Char(':');
    args << ownerSpec << partition().mountPoint();

    ExternalCommand chown(*report, QStringLiteral("chown"), args);
    bool rval = chown.run() && chown.exitCode() == 0;

    jobFinished(*report, rval);

    return rval;
}

QString TakeOwnershipJob::description() const
{
    if (m_Recursive)
        return xi18nc("@info:progress", "Taking ownership of <filename>%1</filename> and its contents for user %2",
                      partition().mountPoint(), m_UserName);
    return xi18nc("@info:progress", "Taking ownership of <filename>%1</filename> for user %2",
                  partition().mountPoint(), m_UserName);
}
