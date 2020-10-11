/*
    SPDX-FileCopyrightText: 2008-2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2009 Andrew Coles <andrew.i.coles@googlemail.com>
    SPDX-FileCopyrightText: 2014-2020 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2018 Huzaifa Faruqui <huzaifafaruqui@gmail.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "jobs/job.h"

#include "core/device.h"
#include "core/copysource.h"
#include "core/copytarget.h"
#include "core/copysourcedevice.h"
#include "core/copytargetdevice.h"

#include "util/externalcommand.h"
#include "util/report.h"

#include <QIcon>
#include <QTime>
#include <QVariantMap>

#include <KLocalizedString>

Job::Job() :
    m_Report(nullptr),
    m_Status(Status::Pending)
{
}

bool Job::copyBlocks(Report& report, CopyTarget& target, CopySource& source)
{
    m_Report = &report;
    ExternalCommand copyCmd;
    connect(&copyCmd, &ExternalCommand::progress, this, &Job::progress, Qt::QueuedConnection);
    connect(&copyCmd, &ExternalCommand::reportSignal, this, &Job::updateReport, Qt::QueuedConnection);
    return copyCmd.copyBlocks(source, target);
}

bool Job::rollbackCopyBlocks(Report& report, CopyTarget& origTarget, CopySource& origSource)
{
    if (!origSource.overlaps(origTarget)) {
        report.line() << xi18nc("@info:progress", "Source and target for copying do not overlap: Rollback is not required.");
        return true;
    }

    try {
        CopySourceDevice& csd = dynamic_cast<CopySourceDevice&>(origSource);
        CopyTargetDevice& ctd = dynamic_cast<CopyTargetDevice&>(origTarget);

        // default: use values as if we were copying from front to back.
        qint64 undoSourceFirstByte = origTarget.firstByte();
        qint64 undoSourceLastByte = origTarget.firstByte() + origTarget.bytesWritten() - 1;

        qint64 undoTargetFirstByte = origSource.firstByte();
        qint64 undoTargetLastByte = origSource.firstByte() + origTarget.bytesWritten() - 1;

        if (origTarget.firstByte() > origSource.firstByte()) {
            // we were copying from back to front
            undoSourceFirstByte = origTarget.firstByte() + origSource.length() - origTarget.bytesWritten();
            undoSourceLastByte = origTarget.firstByte() + origSource.length() - 1;

            undoTargetFirstByte = origSource.lastByte() - origTarget.bytesWritten() + 1;
            undoTargetLastByte = origSource.lastByte();
        }

        report.line() << xi18nc("@info:progress", "Rollback from: First byte: %1, last byte: %2.", undoSourceFirstByte, undoSourceLastByte);
        report.line() << xi18nc("@info:progress", "Rollback to: First byte: %1, last byte: %2.", undoTargetFirstByte, undoTargetLastByte);

        CopySourceDevice undoSource(ctd.device(), undoSourceFirstByte, undoSourceLastByte);
        if (!undoSource.open()) {
            report.line() << xi18nc("@info:progress", "Could not open device <filename>%1</filename> to rollback copying.", ctd.device().deviceNode());
            return false;
        }

        CopyTargetDevice undoTarget(csd.device(), undoTargetFirstByte, undoTargetLastByte);
        if (!undoTarget.open()) {
            report.line() << xi18nc("@info:progress", "Could not open device <filename>%1</filename> to rollback copying.", csd.device().deviceNode());
            return false;
        }

        return copyBlocks(report, undoTarget, undoSource);
    } catch (...) {
        report.line() << xi18nc("@info:progress", "Rollback failed: Source or target are not devices.");
    }

    return false;
}

void Job::emitProgress(int i)
{
    Q_EMIT progress(i);
}

void Job::updateReport(const QString& report)
{
    m_Report->line() << report;
}

Report* Job::jobStarted(Report& parent)
{
    Q_EMIT started();

    return parent.newChild(xi18nc("@info:progress", "Job: %1", description()));
}

void Job::jobFinished(Report& report, bool b)
{
    setStatus(b ? Status::Success : Status::Error);
    Q_EMIT progress(numSteps());
    Q_EMIT finished();

    report.setStatus(xi18nc("@info:progress job status (error, warning, ...)", "%1: %2", description(), statusText()));
}

/** @return the Job's current status icon */
QString Job::statusIcon() const
{
    static const QString icons[] = {
        QStringLiteral("dialog-information"),
        QStringLiteral("dialog-ok"),
        QStringLiteral("dialog-error")
    };

    return icons[static_cast<int>(status())];
}

/** @return the Job's current status text */
QString Job::statusText() const
{
    static const QString s[] = {
        xi18nc("@info:progress job", "Pending"),
        xi18nc("@info:progress job", "Success"),
        xi18nc("@info:progress job", "Error")
    };

    return s[static_cast<int>(status())];
}
