/*
    SPDX-FileCopyrightText: 2026 Ramil Nurmanov <ramil2004nur@gmail.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "jobs/trimjob.h"

#include "core/partition.h"

#include "ops/trimoperation.h"

#include "util/capacity.h"
#include "util/report.h"

#include "externalcommandhelper_interface.h"

#include <QDBusConnection>
#include <QDBusPendingReply>
#include <QVariantMap>

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

    if (!QDBusConnection::systemBus().isConnected()) {
        report->line() << QDBusConnection::systemBus().lastError().message();
        jobFinished(*report, false);
        return false;
    }

    org::kde::kpmcore::externalcommand interface(QStringLiteral("org.kde.kpmcore.helperinterface"),
                                                  QStringLiteral("/Helper"), QDBusConnection::systemBus());
    interface.setTimeout(10 * 24 * 3600 * 1000);

    QDBusPendingReply<QVariantMap> pendingReply = interface.TrimFileSystem(mountPoint, TrimOperation::trimDeviceNode(partition()));
    pendingReply.waitForFinished();

    if (pendingReply.isError()) {
        report->line() << pendingReply.error().message();
        jobFinished(*report, false);
        return false;
    }

    const QVariantMap reply = pendingReply.value();
    const bool rval = reply[QStringLiteral("success")].toBool();

    if (rval)
        report->line() << xi18nc("@info:status", "The TRIM request for <filename>%1</filename> completed successfully. The file system reported %2 as discarded.",
                                 mountPoint, Capacity::formatByteSize(reply[QStringLiteral("trimmed")].toLongLong()));
    else if (reply[QStringLiteral("unsupported")].toBool())
        report->line() << xi18nc("@info:status", "Discarding unused blocks is not supported on <filename>%1</filename>.", mountPoint);
    else
        report->line() << xi18nc("@info:status", "Could not discard unused blocks on <filename>%1</filename>: %2", mountPoint, reply[QStringLiteral("error")].toString());

    jobFinished(*report, rval);

    return rval;
}

QString TrimJob::description() const
{
    return xi18nc("@info:progress", "Discarding unused blocks on <filename>%1</filename>", partition().mountPoint());
}
