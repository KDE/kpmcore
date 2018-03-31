/*************************************************************************
 *  Copyright (C) 2008 by Volker Lanz <vl@fidra.de>                      *
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

#include "backend/corebackendmanager.h"
#include "core/device.h"
#include "core/copysource.h"
#include "core/copytarget.h"
#include "core/copysourcedevice.h"
#include "core/copytargetdevice.h"
#include "util/externalcommand.h"
#include "util/report.h"

#include <QDBusInterface>
#include <QDBusReply>
#include <QEventLoop>
#include <QtGlobal>
#include <QStandardPaths>
#include <QString>
#include <QStringList>
#include <QTimer>
#include <QThread>
#include <QVariant>

#include <KAuth>
#include <KJob>
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

/** Executes the external command.
    @param timeout timeout to wait for the process to start
    @return true on success
*/
bool ExternalCommand::start(int timeout)
{
    Q_UNUSED(timeout)

    if (report()) {
        report()->setCommand(xi18nc("@info:status", "Command: %1 %2", command(), args().join(QStringLiteral(" "))));
    }

    QString cmd = QStandardPaths::findExecutable(command());
    if (cmd.isEmpty())
        cmd = QStandardPaths::findExecutable(command(), { QStringLiteral("/sbin/"), QStringLiteral("/usr/sbin/"), QStringLiteral("/usr/local/sbin/") });

    if (!QDBusConnection::systemBus().isConnected()) {
        qWarning() << "Could not connect to DBus system bus";
        return false;
    }

    QDBusInterface iface(QStringLiteral("org.kde.kpmcore.helperinterface"),
                         QStringLiteral("/Helper"),
                         QStringLiteral("org.kde.kpmcore.externalcommand"),
                         QDBusConnection::systemBus());

    iface.setTimeout(10 * 24 * 3600 * 1000); // 10 days

    bool rval = false;
    if (iface.isValid()) {
        QDBusPendingCall pcall = iface.asyncCall(QStringLiteral("start"),
                                                 CoreBackendManager::self()->Uuid(),
                                                 cmd,
                                                 args(),
                                                 m_Input,
                                                 QStringList());

        QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(pcall, this);

        QEventLoop loop;

        auto exitLoop = [&] (QDBusPendingCallWatcher *watcher) {
            loop.exit();

            if (watcher->isError())
                qWarning() << watcher->error();
            else {
                QDBusPendingReply<QVariantMap> reply = *watcher;

                m_Output = reply.value()[QStringLiteral("output")].toByteArray();
                setExitCode(reply.value()[QStringLiteral("exitCode")].toInt());
                rval = true;
            }
        };

        connect(watcher, &QDBusPendingCallWatcher::finished, exitLoop);
        loop.exec();
    }

    return rval;
}

bool ExternalCommand::copyBlocks(CopySource& source, CopyTarget& target)
{
    bool rval = true;
    const qint64 blockSize = 10 * 1024 * 1024; // number of bytes per block to copy

    if (!QDBusConnection::systemBus().isConnected()) {
        qWarning() << "Could not connect to DBus system bus";
        return false;
    }

    // TODO KF6:Use new signal-slot syntax
    connect(CoreBackendManager::self()->job(), SIGNAL(percent(KJob*, unsigned long)), this, SLOT(emitProgress(KJob*, unsigned long)));
    connect(CoreBackendManager::self()->job(), &KAuth::ExecuteJob::newData, this, &ExternalCommand::emitReport);

    QDBusInterface iface(QStringLiteral("org.kde.kpmcore.helperinterface"), QStringLiteral("/Helper"), QStringLiteral("org.kde.kpmcore.externalcommand"), QDBusConnection::systemBus());
    iface.setTimeout(10 * 24 * 3600 * 1000); // 10 days
    if (iface.isValid()) {
        // Use asynchronous DBus calls, so that we can process reports and progress
        QDBusPendingCall pcall= iface.asyncCall(QStringLiteral("copyblocks"),
                                                CoreBackendManager::self()->Uuid(),
                                                source.path(), source.firstByte(), source.length(),
                                                target.path(), target.firstByte(), blockSize);

        QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(pcall, this);
        QEventLoop loop;

        auto exitLoop = [&] (QDBusPendingCallWatcher *watcher) {
            loop.exit();
            if (watcher->isError()) {
                qWarning() << watcher->error();
            }
            else {
                QDBusPendingReply<bool> reply = *watcher;
                rval = reply.argumentAt<0>();
            }
            setExitCode(!rval);
        };

        connect(watcher, &QDBusPendingCallWatcher::finished, exitLoop);
        loop.exec();
    }

    return rval;
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
    Q_UNUSED(timeout)
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
