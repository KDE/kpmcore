/*
    SPDX-FileCopyrightText: 2008-2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2014-2020 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2015 Teo Mrnjavac <teo@kde.org>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "core/operationrunner.h"
#include "core/operationstack.h"
#include "ops/operation.h"
#include "util/report.h"

#include <QDBusInterface>
#include <QDBusReply>
#include <QMutex>

/** Constructs an OperationRunner.
    @param ostack the OperationStack to act on
*/
OperationRunner::OperationRunner(QObject* parent, OperationStack& ostack) :
    QThread(parent),
    m_OperationStack(ostack),
    m_Report(nullptr),
    m_SuspendMutex(),
    m_Cancelling(false)
{
}

/** Runs the operations in the OperationStack. */
void OperationRunner::run()
{
    Q_ASSERT(m_Report);

    setCancelling(false);

    bool status = true;

    // Disable Plasma removable device automounting
    QStringList modules;
    QDBusConnection bus = QDBusConnection::connectToBus(QDBusConnection::SessionBus, QStringLiteral("sessionBus"));
    QDBusInterface kdedInterface( QStringLiteral("org.kde.kded5"), QStringLiteral("/kded"), QStringLiteral("org.kde.kded5"), bus );
    QDBusReply<QStringList> reply = kdedInterface.call( QStringLiteral("loadedModules")  );
    if ( reply.isValid() )
        modules = reply.value();
    QString automounterService = QStringLiteral("device_automounter");
    bool automounter = modules.contains(automounterService);
    if (automounter)
        kdedInterface.call( QStringLiteral("unloadModule"), automounterService );

    for (int i = 0; i < numOperations(); i++) {
        suspendMutex().lock();
        suspendMutex().unlock();

        if (!status || isCancelling()) {
            break;
        }

        Operation* op = operationStack().operations()[i];
        op->setStatus(Operation::StatusRunning);

        Q_EMIT opStarted(i + 1, op);

        connect(op, &Operation::progress, this, &OperationRunner::progressSub);

        status = op->execute(report());
        op->preview();

        disconnect(op, &Operation::progress, this, &OperationRunner::progressSub);

        Q_EMIT opFinished(i + 1, op);
    }

    if (automounter)
        kdedInterface.call( QStringLiteral("loadModule"), automounterService );

    if (!status)
        Q_EMIT error();
    else if (isCancelling())
        Q_EMIT cancelled();
    else
        Q_EMIT finished();
}

/** @return the number of Operations to run */
qint32 OperationRunner::numOperations() const
{
    return operationStack().operations().size();
}

/** @return the number of Jobs to run */
qint32 OperationRunner::numJobs() const
{
    qint32 result = 0;

    for (const auto &op :  operationStack().operations())
        result += op->jobs().size();

    return result;
}

/** @param op the number of the Operation to get a description for
    @return the Operation's description
*/
QString OperationRunner::description(qint32 op) const
{
    Q_ASSERT(op >= 0);
    Q_ASSERT(op < operationStack().size());

    return operationStack().operations()[op]->description();
}
