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
#include "core/copytargetbytearray.h"
#include "core/copysourcedevice.h"
#include "core/copytargetdevice.h"
#include "util/globallog.h"
#include "util/externalcommand.h"
#include "util/report.h"

#include "externalcommandhelper_interface.h"

#include <QCryptographicHash>
#include <QDBusConnection>
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

struct ExternalCommandPrivate
{
    Report *m_Report;
    QString m_Command;
    QStringList m_Args;
    int m_ExitCode;
    QByteArray m_Output;
    QByteArray m_Input;
    QProcess::ProcessChannelMode processChannelMode;
};

KAuth::ExecuteJob* ExternalCommand::m_job;
QWidget* ExternalCommand::parent;


/** Creates a new ExternalCommand instance without Report.
    @param cmd the command to run
    @param args the arguments to pass to the command
*/
ExternalCommand::ExternalCommand(const QString& cmd, const QStringList& args, const QProcess::ProcessChannelMode processChannelMode) :
    d(std::make_unique<ExternalCommandPrivate>())
{
    d->m_Report = nullptr;
    d->m_Command = cmd;
    d->m_Args = args;
    d->m_ExitCode = -1;
    d->m_Output = QByteArray();
    d->processChannelMode = processChannelMode;
}

/** Creates a new ExternalCommand instance with Report.
    @param report the Report to write output to.
    @param cmd the command to run
    @param args the arguments to pass to the command
 */
ExternalCommand::ExternalCommand(Report& report, const QString& cmd, const QStringList& args, const QProcess::ProcessChannelMode processChannelMode) :
    d(std::make_unique<ExternalCommandPrivate>())
{
    d->m_Report = report.newChild();
    d->m_Command = cmd;
    d->m_Args = args;
    d->m_ExitCode = -1;
    d->m_Output = QByteArray();

    d->processChannelMode = processChannelMode;
}

ExternalCommand::~ExternalCommand()
{
    
}

/*
void ExternalCommand::setup()
{
     connect(this, qOverload<int, QProcess::ExitStatus>(&QProcess::finished), this, &ExternalCommand::onFinished);
     connect(this, &ExternalCommand::readyReadStandardOutput, this, &ExternalCommand::onReadOutput);
}
*/

/** Executes the external command.
    @param timeout timeout to wait for the process to start
    @return true on success
*/
bool ExternalCommand::start(int timeout)
{
    Q_UNUSED(timeout)

    if (command().isEmpty())
        return false;

    if (!QDBusConnection::systemBus().isConnected()) {
        qWarning() << QDBusConnection::systemBus().lastError().message();
        return false;
    }

    if (report())
        report()->setCommand(xi18nc("@info:status", "Command: %1 %2", command(), args().join(QStringLiteral(" "))));

    if ( qEnvironmentVariableIsSet( "KPMCORE_DEBUG" ))
        qDebug() << xi18nc("@info:status", "Command: %1 %2", command(), args().join(QStringLiteral(" ")));

    QString cmd = QStandardPaths::findExecutable(command());
    if (cmd.isEmpty())
        cmd = QStandardPaths::findExecutable(command(), { QStringLiteral("/sbin/"), QStringLiteral("/usr/sbin/"), QStringLiteral("/usr/local/sbin/") });

    auto *interface = new org::kde::kpmcore::externalcommand(QStringLiteral("org.kde.kpmcore.helperinterface"),
                    QStringLiteral("/Helper"), QDBusConnection::systemBus(), this);

    interface->setTimeout(10 * 24 * 3600 * 1000); // 10 days

    if (!interface)
        return false;

    bool rval = false;

    QDBusPendingCall pcall = interface->start(cmd, args(), d->m_Input, d->processChannelMode);

    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(pcall, this);
    QEventLoop loop;

    auto exitLoop = [&] (QDBusPendingCallWatcher *watcher) {
        loop.exit();

        if (watcher->isError())
            qWarning() << watcher->error();
        else {
            QDBusPendingReply<QVariantMap> reply = *watcher;

            d->m_Output = reply.value()[QStringLiteral("output")].toByteArray();
            setExitCode(reply.value()[QStringLiteral("exitCode")].toInt());
            rval = reply.value()[QStringLiteral("success")].toBool();
        }
    };

    connect(watcher, &QDBusPendingCallWatcher::finished, exitLoop);
    loop.exec();

    return rval;
}

bool ExternalCommand::copyBlocks(const CopySource& source, CopyTarget& target)
{
    bool rval = true;
    const qint64 blockSize = 10 * 1024 * 1024; // number of bytes per block to copy

    if (!QDBusConnection::systemBus().isConnected()) {
        qWarning() << QDBusConnection::systemBus().lastError().message();
        return false;
    }

    auto *interface = new org::kde::kpmcore::externalcommand(QStringLiteral("org.kde.kpmcore.externalcommand"),
                QStringLiteral("/Helper"), QDBusConnection::systemBus(), this);
    interface->setTimeout(10 * 24 * 3600 * 1000); // 10 days

    if (!interface)
        return false;

    connect(interface, &OrgKdeKpmcoreExternalcommandInterface::progress, this, &ExternalCommand::progress);
    connect(interface, &OrgKdeKpmcoreExternalcommandInterface::report, this, &ExternalCommand::reportSignal);

    QDBusPendingCall pcall = interface->copyblocks(source.path(), source.firstByte(), source.length(),
                                                   target.path(), target.firstByte(), blockSize);

    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(pcall, this);
    QEventLoop loop;

    auto exitLoop = [&] (QDBusPendingCallWatcher *watcher) {
        loop.exit();
        if (watcher->isError())
            qWarning() << watcher->error();
        else {
            QDBusPendingReply<QVariantMap> reply = *watcher;
            rval = reply.value()[QStringLiteral("success")].toBool();

            CopyTargetByteArray *byteArrayTarget = dynamic_cast<CopyTargetByteArray*>(&target);
            if (byteArrayTarget)
                byteArrayTarget->m_Array = reply.value()[QStringLiteral("targetByteArray")].toByteArray();
        }
        setExitCode(!rval);
    };

    connect(watcher, &QDBusPendingCallWatcher::finished, exitLoop);
    loop.exec();

    return rval;
}

bool ExternalCommand::writeData(Report& commandReport, const QByteArray& buffer, const QString& deviceNode, const quint64 firstByte)
{
    d->m_Report = commandReport.newChild();
    if (report())
        report()->setCommand(xi18nc("@info:status", "Command: %1 %2", command(), args().join(QStringLiteral(" "))));

    bool rval = true;

    if (!QDBusConnection::systemBus().isConnected()) {
        qWarning() << QDBusConnection::systemBus().lastError().message();
        return false;
    }

    auto *interface = new org::kde::kpmcore::externalcommand(QStringLiteral("org.kde.kpmcore.helperinterface"),
                QStringLiteral("/Helper"), QDBusConnection::systemBus(), this);
    interface->setTimeout(10 * 24 * 3600 * 1000); // 10 days
 
    QDBusPendingCall pcall = interface->writeData(buffer, deviceNode, firstByte);

    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(pcall, this);
    QEventLoop loop;

    auto exitLoop = [&] (QDBusPendingCallWatcher *watcher) {
        loop.exit();
        if (watcher->isError())
            qWarning() << watcher->error();
        else {
            QDBusPendingReply<bool> reply = *watcher;
            rval = reply.argumentAt<0>();
        }
        setExitCode(!rval);
    };

    connect(watcher, &QDBusPendingCallWatcher::finished, exitLoop);
    loop.exec();

    return rval;
}


bool ExternalCommand::write(const QByteArray& input)
{
    if ( qEnvironmentVariableIsSet( "KPMCORE_DEBUG" ))
        qDebug() << "Command input:" << QString::fromLocal8Bit(input);
    d->m_Input = input;
    return true;
}

/** Runs the command.
    @param timeout timeout to use for waiting when starting and when waiting for the process to finish
    @return true on success
*/
bool ExternalCommand::run(int timeout)
{
    return start(timeout) /* && exitStatus() == 0*/;
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

void ExternalCommand::setCommand(const QString& cmd)
{
    d->m_Command = cmd;
}

const QString& ExternalCommand::command() const
{
    return d->m_Command;
}

const QStringList& ExternalCommand::args() const
{
    return d->m_Args;
}

void ExternalCommand::addArg(const QString& s)
{
    d->m_Args << s;
}

void ExternalCommand::setArgs(const QStringList& args)
{
    d->m_Args = args;
}

int ExternalCommand::exitCode() const
{
    return d->m_ExitCode;
}

const QString ExternalCommand::output() const
{
    return QString::fromLocal8Bit(d->m_Output);
}

const QByteArray& ExternalCommand::rawOutput() const
{
    return d->m_Output;
}

Report* ExternalCommand::report()
{
    return d->m_Report;
}

void ExternalCommand::setExitCode(int i)
{
    d->m_ExitCode = i;
}

void ExternalCommand::stopHelper()
{
}
