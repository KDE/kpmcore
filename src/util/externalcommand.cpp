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

#include "backend/corebackendmanager.h"
#include "util/externalcommand.h"
#include "util/report.h"

#include <QDebug>
#include <QEventLoop>
#include <QtGlobal>
#include <QStandardPaths>
#include <QString>
#include <QStringList>
#include <QTimer>
#include <QThread>

#include <KAuth>
#include <KLocalizedString>

/** Creates a new ExternalCommand instance without Report.
    @param cmd the command to run
    @param args the arguments to pass to the command
*/
ExternalCommand::ExternalCommand(const QString& cmd, const QStringList& args, const QProcess::ProcessChannelMode processChannelMode) :
    m_Report(nullptr),
    m_Command(cmd),
    m_Args(args),
    m_ExitCode(-1),
    m_Output()
{
    setup(processChannelMode);
}

/** Creates a new ExternalCommand instance with Report.
    @param report the Report to write output to.
    @param cmd the command to run
    @param args the arguments to pass to the command
 */
ExternalCommand::ExternalCommand(Report& report, const QString& cmd, const QStringList& args, const QProcess::ProcessChannelMode processChannelMode) :
    m_Report(report.newChild()),
    m_Command(cmd),
    m_Args(args),
    m_ExitCode(-1),
    m_Output()
{
    setup(processChannelMode);
}

void ExternalCommand::setup(const QProcess::ProcessChannelMode processChannelMode)
{
    arguments.insert(QStringLiteral("environment"), QStringList() << QStringLiteral("LC_ALL=C") << QStringLiteral("LVM_SUPPRESS_FD_WARNINGS=1"));
    arguments.insert(QStringLiteral("processChannelMode"), processChannelMode);

//     connect(this, qOverload<int, QProcess::ExitStatus>(&QProcess::finished), this, &ExternalCommand::onFinished);
//     connect(this, &ExternalCommand::readyReadStandardOutput, this, &ExternalCommand::onReadOutput);
}

/** Starts the external command.
    @param timeout timeout to wait for the process to start
    @return true on success
*/
bool ExternalCommand::start(int timeout)
{
    this->moveToThread(CoreBackendManager::self()->kauthThread());
    QTimer::singleShot(0, this, &ExternalCommand::execute);
    QEventLoop loop;
    connect(this, &ExternalCommand::finished, &loop, &QEventLoop::quit);
    loop.exec();
    return true;
}

/** Executes the external command in kauthThread() thread.
*/
void ExternalCommand::execute()
{
    if (report()) {
        report()->setCommand(xi18nc("@info:status", "Command: %1 %2", command(), args().join(QStringLiteral(" "))));
    }

    QString cmd = QStandardPaths::findExecutable(command());
    if (cmd.isEmpty())
        cmd = QStandardPaths::findExecutable(command(), { QStringLiteral("/sbin/"), QStringLiteral("/usr/sbin/"), QStringLiteral("/usr/local/sbin/") });

    KAuth::Action action(QStringLiteral("org.kde.kpmcore.externalcommand.start"));
    action.setHelperId(QStringLiteral("org.kde.kpmcore.externalcommand"));
    arguments.insert(QStringLiteral("command"), cmd);
    arguments.insert(QStringLiteral("input"), m_Input);
    arguments.insert(QStringLiteral("arguments"), args());
    action.setArguments(arguments);

    KAuth::ExecuteJob *job = action.execute();
    if (!job->exec()) {
        qWarning() << "KAuth returned an error code: " << job->errorString();
//         return false;
        emit finished();
        return;
    }

    m_Output = job->data()[QStringLiteral("output")].toByteArray();
    setExitCode(job->data()[QStringLiteral("exitCode")].toInt());

//     QProcess::start(command(), args());

    // FIXME
//     if (!waitForStarted(timeout))
//     {
//         if (report())
//             report()->line() << xi18nc("@info:status", "(Command timeout while starting)");
//         return false;
//     }

//     return true;
    emit finished();
}

bool ExternalCommand::write(const QByteArray& input)
{
    m_Input = input;
    return true;
}

/** Waits for the external command to finish.
    @param timeout timeout to wait until the process finishes.
    @return true on success
*/
bool ExternalCommand::waitFor(int timeout)
{
//     closeWriteChannel();
/*
    if (!waitForFinished(timeout)) {
        if (report())
            report()->line() << xi18nc("@info:status", "(Command timeout while running)");
        return false;
    }*/

//     onReadOutput();
    return true;
}

/** Runs the command.
    @param timeout timeout to use for waiting when starting and when waiting for the process to finish
    @return true on success
*/
bool ExternalCommand::run(int timeout)
{
    return start(timeout) && waitFor(timeout)/* && exitStatus() == 0*/;
}

void ExternalCommand::onReadOutput()
{
//     const QByteArray s = readAllStandardOutput();
//
//     if(m_Output.length() > 10*1024*1024) { // prevent memory overflow for badly corrupted file systems
//         if (report())
//             report()->line() << xi18nc("@info:status", "(Command is printing too much output)");
//         return;
//     }
//
//     m_Output += s;
//
//     if (report())
//         *report() << QString::fromLocal8Bit(s);
}

void ExternalCommand::onFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitStatus)
    setExitCode(exitCode);
}
