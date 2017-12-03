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

#include "util/externalcommand.h"

#include "util/report.h"

#include <QtGlobal>
#include <QString>
#include <QStringList>

#include <KLocalizedString>

/** Creates a new ExternalCommand instance without Report.
    @param cmd the command to run
    @param args the arguments to pass to the command
*/
ExternalCommand::ExternalCommand(const QString& cmd, const QStringList& args, const QProcess::ProcessChannelMode processChannelMode) :
    QProcess(),
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
    QProcess(),
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
    setEnvironment(QStringList() << QStringLiteral("LC_ALL=C") << QStringLiteral("PATH=") + QString::fromLocal8Bit(getenv("PATH")) << QStringLiteral("LVM_SUPPRESS_FD_WARNINGS=1"));
    setProcessChannelMode(processChannelMode);

    connect(this, qOverload<int, QProcess::ExitStatus>(&QProcess::finished), this, &ExternalCommand::onFinished);
    connect(this, &ExternalCommand::readyReadStandardOutput, this, &ExternalCommand::onReadOutput);
}

/** Starts the external command.
    @param timeout timeout to wait for the process to start
    @return true on success
*/
bool ExternalCommand::start(int timeout)
{
    QProcess::start(command(), args());

    if (report()) {
        report()->setCommand(xi18nc("@info:status", "Command: %1 %2", command(), args().join(QStringLiteral(" "))));
    }

    if (!waitForStarted(timeout))
    {
        if (report())
            report()->line() << xi18nc("@info:status", "(Command timeout while starting)");
        return false;
    }

    return true;
}

/** Waits for the external command to finish.
    @param timeout timeout to wait until the process finishes.
    @return true on success
*/
bool ExternalCommand::waitFor(int timeout)
{
    closeWriteChannel();

    if (!waitForFinished(timeout)) {
        if (report())
            report()->line() << xi18nc("@info:status", "(Command timeout while running)");
        return false;
    }

    onReadOutput();
    return true;
}

/** Runs the command.
    @param timeout timeout to use for waiting when starting and when waiting for the process to finish
    @return true on success
*/
bool ExternalCommand::run(int timeout)
{
    return start(timeout) && waitFor(timeout) && exitStatus() == 0;
}

void ExternalCommand::onReadOutput()
{
    const QByteArray s = readAllStandardOutput();

    if(m_Output.length() > 10*1024*1024) { // prevent memory overflow for badly corrupted file systems
        if (report())
            report()->line() << xi18nc("@info:status", "(Command is printing too much output)");
        return;
    }

    m_Output += s;

    if (report())
        *report() << QString::fromLocal8Bit(s);
}

void ExternalCommand::onFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitStatus)
    setExitCode(exitCode);
}
