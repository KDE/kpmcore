/*************************************************************************
 *  Copyright (C) 2008 by Volker Lanz <vl@fidra.de>                      *
 *  Copyright (C) 2016 by Andrius Štikonas <andrius@stikonas.eu>         *
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

#include "ops/operation_p.h"

#include "core/partition.h"
#include "core/device.h"

#include "jobs/job.h"

#include "util/report.h"

#include <QDebug>
#include <QIcon>
#include <QString>

#include <KLocalizedString>

Operation::Operation() :
    d(std::make_unique<OperationPrivate>())
{
    d->m_Status = StatusNone;
    d->m_ProgressBase = 0;
}

Operation::~Operation()
{
    qDeleteAll(jobs());
    jobs().clear();
}

void Operation::insertPreviewPartition(Device& device, Partition& p)
{
    Q_ASSERT(device.partitionTable());

    device.partitionTable()->removeUnallocated();

    p.parent()->insert(&p);

    device.partitionTable()->updateUnallocated(device);
}

void Operation::removePreviewPartition(Device& device, Partition& p)
{
    Q_ASSERT(device.partitionTable());

    if (p.parent()->remove(&p))
        device.partitionTable()->updateUnallocated(device);
    else
        qWarning() << "failed to remove partition " << p.deviceNode() << " at " << &p << " from preview.";
}

/** @return text describing the Operation's current status */
QString Operation::statusText() const
{
    static const QString s[] = {
        xi18nc("@info:progress operation", "None"),
        xi18nc("@info:progress operation", "Pending"),
        xi18nc("@info:progress operation", "Running"),
        xi18nc("@info:progress operation", "Success"),
        xi18nc("@info:progress operation", "Warning"),
        xi18nc("@info:progress operation", "Error")
    };

    Q_ASSERT(status() >= 0 && static_cast<quint32>(status()) < sizeof(s) / sizeof(s[0]));

    if (status() < 0 || static_cast<quint32>(status()) >= sizeof(s) / sizeof(s[0])) {
        qWarning() << "invalid status " << status();
        return QString();
    }

    return s[status()];
}

/** @return icon for the current Operation's status */
QString Operation::statusIcon() const
{
    static const QString icons[] = {
        QString(),
        QStringLiteral("dialog-information"),
        QStringLiteral("dialog-information"),
        QStringLiteral("dialog-ok"),
        QStringLiteral("dialog-warning"),
        QStringLiteral("dialog-error")
    };

    Q_ASSERT(status() >= 0 && static_cast<quint32>(status()) < sizeof(icons) / sizeof(icons[0]));

    if (status() < 0 || static_cast<quint32>(status()) >= sizeof(icons) / sizeof(icons[0])) {
        qWarning() << "invalid status " << status();
        return QString();
    }

    if (status() == StatusNone)
        return QString();

    return icons[status()];
}

void Operation::addJob(Job* job)
{
    if (job) {
        jobs().append(job);
        connect(job, &Job::started, this, &Operation::onJobStarted);
        connect(job, &Job::progress, this, &Operation::progress);
        connect(job, &Job::finished, this, &Operation::onJobFinished);
    }
}

void Operation::onJobStarted()
{
    Job* job = qobject_cast<Job*>(sender());

    if (job)
        emit jobStarted(job, this);
}

void Operation::onJobFinished()
{
    Job* job = qobject_cast<Job*>(sender());

    if (job) {
        setProgressBase(progressBase() + job->numSteps());
        emit jobFinished(job, this);
    }
}

/** @return total number of steps to run this Operation */
qint32 Operation::totalProgress() const
{
    qint32 result = 0;

    for (const auto &job : jobs())
        result += job->numSteps();

    return result;
}

/** Execute the operation
    @param parent the parent Report to create a new child for
    @return true on success
*/
bool Operation::execute(Report& parent)
{
    bool rval = false;

    Report* report = parent.newChild(description());

    const auto Jobs = jobs();
    for (const auto &job : Jobs)
        if (!(rval = job->run(*report)))
            break;

    setStatus(rval ? StatusFinishedSuccess : StatusError);

    report->setStatus(xi18nc("@info:status (success, error, warning...) of operation", "%1: %2", description(), statusText()));

    return rval;
}

Operation::OperationStatus Operation::status() const
{
    return d->m_Status;
}

void Operation::setStatus(OperationStatus s)
{
    d->m_Status = s;
}

QList<Job*>& Operation::jobs()
{
    return d->m_Jobs;
}

const QList<Job*>& Operation::jobs() const
{
    return d->m_Jobs;
}

void Operation::setProgressBase(qint32 i)
{
    d->m_ProgressBase = i;
}

qint32 Operation::progressBase() const
{
    return d->m_ProgressBase;
}
