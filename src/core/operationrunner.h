/*
    SPDX-FileCopyrightText: 2008-2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2014-2018 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2019 Albert Astals Cid <aacid@kde.org>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KPMCORE_OPERATIONRUNNER_H
#define KPMCORE_OPERATIONRUNNER_H

#include "util/libpartitionmanagerexport.h"

#include <QThread>
#include <QMutex>
#include <QtGlobal>

class Operation;
class OperationStack;
class Report;

/** Thread to run the Operations in the OperationStack.

    Runs the OperationStack when the user applies operations.

    @author Volker Lanz <vl@fidra.de>
*/
class LIBKPMCORE_EXPORT OperationRunner : public QThread
{
    Q_OBJECT
    Q_DISABLE_COPY(OperationRunner)

public:
    OperationRunner(QObject* parent, OperationStack& ostack);

public:
    void run() override;
    qint32 numJobs() const;
    qint32 numOperations() const;
    qint32 numProgressSub() const;
    bool isCancelling() const {
        return m_Cancelling;    /**< @return if the user has requested cancelling */
    }
    void cancel() const {
        m_Cancelling = true;    /**< Sets cancelling to true. */
    }
    QMutex& suspendMutex() const {
        return m_SuspendMutex;    /**< @return the QMutex used for syncing */
    }
    QString description(qint32 op) const;
    void setReport(Report* report) {
        m_Report = report;    /**< @param report the Report to use while running */
    }

Q_SIGNALS:
    void progressSub(int);
    void opStarted(int, Operation*);
    void opFinished(int, Operation*);
    void finished();
    void cancelled();
    void error();

protected:
    OperationStack& operationStack() {
        return m_OperationStack;
    }
    const OperationStack& operationStack() const {
        return m_OperationStack;
    }
    void setCancelling(bool b) {
        m_Cancelling = b;
    }
    Report& report() {
        Q_ASSERT(m_Report);
        return *m_Report;
    }

private:
    OperationStack& m_OperationStack;
    Report* m_Report;
    mutable QMutex m_SuspendMutex;
    mutable volatile bool m_Cancelling;
};

#endif
