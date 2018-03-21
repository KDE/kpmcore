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
#include <KLocalizedString>


ExternalCommand::ExternalCommand(CopySource& source, CopyTarget& target,const QProcess::ProcessChannelMode processChannelMode) :
   m_ExitCode(-1),
   m_Source(&source),
   m_Target(&target)
{
    setup(processChannelMode);
}

/** Starts copyBlocks command.
*/
bool ExternalCommand::startCopyBlocks()
{
    this->moveToThread(CoreBackendManager::self()->kauthThread());
    QTimer::singleShot(0, this, &ExternalCommand::copyBlocks);
    QEventLoop loop;
    connect(this, &ExternalCommand::finished, &loop, &QEventLoop::quit);
    loop.exec();
    return true;
}

bool ExternalCommand::copyBlocks()
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
    if (iface.isValid()) {
        QDBusReply<QVariantMap> reply = iface.call(QStringLiteral("copyblocks"), CoreBackendManager::self()->Uuid(), m_Source->path(), m_Source->firstByte(), m_Source->length(), m_Target->path(), m_Target->firstByte(), blockSize);
        if (reply.isValid()) {
            rval = reply.value()[QStringLiteral("success")].toInt();
            qDebug() << rval;
        }
        else {
            qWarning() << reply.error().message();
        }
    }

    emit finished();
    setExitCode(!rval);
    return rval;
}

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
//     this->moveToThread(CoreBackendManager::self()->kauthThread());
//     QTimer::singleShot(0, this, &ExternalCommand::execute);
//     QEventLoop loop;
//     connect(this, &ExternalCommand::finished, &loop, &QEventLoop::quit);
//     loop.exec();
//     return true;
    execute();
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

    if (!QDBusConnection::systemBus().isConnected()) {
        qWarning() << "Could not connect to DBus system bus";
        return;
    }

    QDBusInterface iface(QStringLiteral("org.kde.kpmcore.helperinterface"), QStringLiteral("/Helper"), QStringLiteral("org.kde.kpmcore.externalcommand"), QDBusConnection::systemBus());
    if (iface.isValid()) {
        QDBusReply<QVariantMap> reply = iface.call(QStringLiteral("start"), CoreBackendManager::self()->Uuid(), cmd, args(), m_Input, QStringList());
        if (reply.isValid()) {
            m_Output = reply.value()[QStringLiteral("output")].toByteArray();
            setExitCode(reply.value()[QStringLiteral("exitCode")].toInt());
        }
        else {
            qWarning() << reply.error().message();
        }
    }

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
