/*
    SPDX-FileCopyrightText: 2008-2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2014-2020 Andrius Štikonas <andrius@stikonas.eu>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "ops/operation_p.h"

#include "core/partition.h"
#include "core/device.h"
#include "core/lvmdevice.h"

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

    if (p.parent()->insert(&p)) {
        if (device.type() == Device::Type::LVM_Device) {
            const LvmDevice& lvm = static_cast<const LvmDevice&>(device);
            lvm.setFreePE(lvm.freePE() - p.length());
        }
    }
    else
        qWarning() << "failed to insert preview partition " << p.deviceNode() << " at " << &p << ".";

    device.partitionTable()->updateUnallocated(device);
}

void Operation::removePreviewPartition(Device& device, Partition& p)
{
    Q_ASSERT(device.partitionTable());

    if (p.parent()->remove(&p)) {
        if (device.type() == Device::Type::LVM_Device) {
            const LvmDevice& lvm = static_cast<const LvmDevice&>(device);
            lvm.setFreePE(lvm.freePE() + p.length());
        }

        device.partitionTable()->updateUnallocated(device);
    }
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
        Q_EMIT jobStarted(job, this);
}

void Operation::onJobFinished()
{
    Job* job = qobject_cast<Job*>(sender());

    if (job) {
        setProgressBase(progressBase() + job->numSteps());
        Q_EMIT jobFinished(job, this);
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

#include "moc_operation.cpp"
