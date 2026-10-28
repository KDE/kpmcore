/*
    SPDX-FileCopyrightText: 2026 Ramil Nurmanov <ramil2004nur@gmail.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "jobs/trimjob.h"

#include "core/partition.h"

#include "ops/trimoperation.h"

#include "util/externalcommand.h"
#include "util/report.h"

#include <KLocalizedString>

TrimJob::TrimJob(Partition& p) :
    Job(),
    m_Partition(p)
{
}

bool TrimJob::run(Report& parent)
{
    Report* report = jobStarted(parent);

    const QString mountPoint = partition().mountPoint();

    switch (TrimOperation::check(partition())) {
    case TrimOperation::Check::Ok:
        break;
    case TrimOperation::Check::ToolNotFound:
        report->line() << xi18nc("@info:status", "Could not find the <command>fstrim</command> program.");
        jobFinished(*report, false);
        return false;
    case TrimOperation::Check::UnsupportedFileSystem:
        report->line() << xi18nc("@info:status", "The file system on <filename>%1</filename> does not support discarding unused blocks.", partition().deviceNode());
        jobFinished(*report, false);
        return false;
    case TrimOperation::Check::NotMounted:
        report->line() << xi18nc("@info:status", "<filename>%1</filename> is no longer mounted at <filename>%2</filename>.", partition().deviceNode(), mountPoint);
        jobFinished(*report, false);
        return false;
    case TrimOperation::Check::ReadOnly:
        report->line() << xi18nc("@info:status", "<filename>%1</filename> is mounted read-only.", mountPoint);
        jobFinished(*report, false);
        return false;
    case TrimOperation::Check::DiscardUnsupported:
        report->line() << xi18nc("@info:status", "The device of <filename>%1</filename> does not support discarding blocks.", partition().deviceNode());
        jobFinished(*report, false);
        return false;
    }

    ExternalCommand cmd(*report, QStringLiteral("fstrim"), { QStringLiteral("--verbose"), QStringLiteral("--"), mountPoint });
    const bool rval = cmd.run(-1) && cmd.exitCode() == 0;

    if (!cmd.output().trimmed().isEmpty())
        *report << cmd.output().trimmed();

    if (rval)
        report->line() << xi18nc("@info:status", "Discarding unused blocks on <filename>%1</filename> completed successfully.", mountPoint);
    else
        report->line() << xi18nc("@info:status", "Could not discard unused blocks on <filename>%1</filename>.", mountPoint);

    jobFinished(*report, rval);

    return rval;
}

QString TrimJob::description() const
{
    return xi18nc("@info:progress", "Discarding unused blocks on <filename>%1</filename>", partition().mountPoint());
}
