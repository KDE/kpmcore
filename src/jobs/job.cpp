/*************************************************************************
 *  Copyright (C) 2008, 2009, 2010 by Volker Lanz <vl@fidra.de>          *
 *  Copyright (C) 2016-2018 by Andrius Štikonas <andrius@stikonas.eu>    *
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
    if (copyCmd.copyBlocks(source, target)) {
        return true;
    }

    return false;
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
    emit progress(i);
}

void Job::updateReport(const QVariantMap& reportString)
{
    m_Report->line() << reportString[QStringLiteral("report")].toString();
}

Report* Job::jobStarted(Report& parent)
{
    emit started();

    return parent.newChild(xi18nc("@info:progress", "Job: %1", description()));
}

void Job::jobFinished(Report& report, bool b)
{
    setStatus(b ? Status::Success : Status::Error);
    emit progress(numSteps());
    emit finished();

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
