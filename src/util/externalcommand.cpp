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

#include <QtCrypto>

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
    DBusThread *m_thread;
    QProcess::ProcessChannelMode processChannelMode;
};

KAuth::ExecuteJob* ExternalCommand::m_job;
QCA::PrivateKey* ExternalCommand::privateKey;
QCA::Initializer* ExternalCommand::init;
bool ExternalCommand::helperStarted = false;
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

    if (!helperStarted)
        if(!startHelper())
            Log(Log::Level::error) << xi18nc("@info:status", "Could not obtain administrator privileges.");

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

// void ExternalCommand::setup()
// {
//     connect(this, qOverload<int, QProcess::ExitStatus>(&QProcess::finished), this, &ExternalCommand::onFinished);
//     connect(this, &ExternalCommand::readyReadStandardOutput, this, &ExternalCommand::onReadOutput);
// }

/** Executes the external command.
    @param timeout timeout to wait for the process to start
    @return true on success
*/
bool ExternalCommand::start(int timeout)
{
    Q_UNUSED(timeout)

    if (command().isEmpty())
        return false;

    if (report())
        report()->setCommand(xi18nc("@info:status", "Command: %1 %2", command(), args().join(QStringLiteral(" "))));

    if ( qEnvironmentVariableIsSet( "KPMCORE_DEBUG" ))
        qDebug() << xi18nc("@info:status", "Command: %1 %2", command(), args().join(QStringLiteral(" ")));

    QString cmd = QStandardPaths::findExecutable(command());
    if (cmd.isEmpty())
        cmd = QStandardPaths::findExecutable(command(), { QStringLiteral("/sbin/"), QStringLiteral("/usr/sbin/"), QStringLiteral("/usr/local/sbin/") });

    if (!QDBusConnection::systemBus().isConnected()) {
        qWarning() << "Could not connect to DBus system bus";
        return false;
    }

    auto *interface = new org::kde::kpmcore::externalcommand(QStringLiteral("org.kde.kpmcore.externalcommand"),
                    QStringLiteral("/Helper"), QDBusConnection::systemBus(), this);

    interface->setTimeout(10 * 24 * 3600 * 1000); // 10 days

    bool rval = false;
    QByteArray request;
    const quint64 nonce = interface->getNonce();
    request.setNum(nonce);
    request.append(cmd.toUtf8());
    for (const auto &argument : qAsConst(d->m_Args))
        request.append(argument.toUtf8());
    request.append(d->m_Input);
    request.append(d->processChannelMode);

    QByteArray hash = QCryptographicHash::hash(request, QCryptographicHash::Sha512);

    QDBusPendingCall pcall = interface->start(privateKey->signMessage(hash, QCA::EMSA3_Raw),
                                             nonce, cmd, args(), d->m_Input, d->processChannelMode);

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

bool ExternalCommand::copyBlocks(CopySource& source, CopyTarget& target)
{
    bool rval = true;
    const qint64 blockSize = 10 * 1024 * 1024; // number of bytes per block to copy

    if (!QDBusConnection::systemBus().isConnected()) {
        qWarning() << "Could not connect to DBus system bus";
        return false;
    }

    // TODO KF6:Use new signal-slot syntax
    connect(m_job, SIGNAL(percent(KJob*, unsigned long)), this, SLOT(emitProgress(KJob*, unsigned long)));
    connect(m_job, &KAuth::ExecuteJob::newData, this, &ExternalCommand::emitReport);

    auto *interface = new org::kde::kpmcore::externalcommand(QStringLiteral("org.kde.kpmcore.externalcommand"),
                QStringLiteral("/Helper"), QDBusConnection::systemBus(), this);
    interface->setTimeout(10 * 24 * 3600 * 1000); // 10 days
    QByteArray request;

    const quint64 nonce = interface->getNonce();
    request.setNum(nonce);
    request.append(source.path().toUtf8());
    request.append(QByteArray::number(source.firstByte()));
    request.append(QByteArray::number(source.length()));
    request.append(target.path().toUtf8());
    request.append(QByteArray::number(target.firstByte()));
    request.append(QByteArray::number(blockSize));

    QByteArray hash = QCryptographicHash::hash(request, QCryptographicHash::Sha512);

    QDBusPendingCall pcall = interface->copyblocks(privateKey->signMessage(hash, QCA::EMSA3_Raw), nonce,
                                            source.path(), source.firstByte(), source.length(),
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
        qWarning() << "Could not connect to DBus system bus";
        return false;
    }

    auto *interface = new org::kde::kpmcore::externalcommand(QStringLiteral("org.kde.kpmcore.externalcommand"),
                QStringLiteral("/Helper"), QDBusConnection::systemBus(), this);
    interface->setTimeout(10 * 24 * 3600 * 1000); // 10 days
    QByteArray request;

    const quint64 nonce = interface->getNonce();
    request.setNum(nonce);
    request.append(buffer);
    request.append(deviceNode.toUtf8());
    request.append(QByteArray::number(firstByte));

    QByteArray hash = QCryptographicHash::hash(request, QCryptographicHash::Sha512);

    QDBusPendingCall pcall = interface->writeData(privateKey->signMessage(hash, QCA::EMSA3_Raw), nonce,
                                            buffer, deviceNode, firstByte);

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

bool ExternalCommand::startHelper()
{
    if (!QDBusConnection::systemBus().isConnected()) {
        qWarning() << "Could not connect to DBus session bus";
        return false;
    }
    QDBusInterface iface(QStringLiteral("org.kde.kpmcore.helperinterface"), QStringLiteral("/Helper"), QStringLiteral("org.kde.kpmcore.externalcommand"), QDBusConnection::systemBus());
    if (iface.isValid()) {
        exit(0);
    }

    d->m_thread = new DBusThread;
    d->m_thread->start();

    init = new QCA::Initializer;
    // Generate RSA key pair for signing external command requests
    if (!QCA::isSupported("pkey") || !QCA::PKey::supportedIOTypes().contains(QCA::PKey::RSA)) {
        qCritical() << xi18n("QCA does not support RSA.");
        return false;
    }

    privateKey = new QCA::PrivateKey;
    *privateKey = QCA::KeyGenerator().createRSA(4096);
    if(privateKey->isNull()) {
        qCritical() << xi18n("Failed to make private RSA key.");
        return false;
    }

    if (!privateKey->canSign()) {
        qCritical() << xi18n("Generated key cannot be used for signatures.");
        return false;
    }

    QCA::PublicKey pubkey = privateKey->toPublicKey();

    KAuth::Action action = KAuth::Action(QStringLiteral("org.kde.kpmcore.externalcommand.init"));
    action.setHelperId(QStringLiteral("org.kde.kpmcore.externalcommand"));
    action.setTimeout(10 * 24 * 3600 * 1000); // 10 days
    action.setParentWidget(parent);
    QVariantMap arguments;
    arguments.insert(QStringLiteral("pubkey"), pubkey.toDER());
    action.setArguments(arguments);
    m_job = action.execute();
    m_job->start();

    // Wait until ExternalCommand Helper is ready (helper sends newData signal just before it enters event loop)
    QEventLoop loop;
    auto exitLoop = [&] () { loop.exit(); };
    auto conn = QObject::connect(m_job, &KAuth::ExecuteJob::newData, exitLoop);
    QObject::connect(m_job, &KJob::finished, [=] () { if(m_job->error()) exitLoop(); } );
    loop.exec();
    QObject::disconnect(conn);

    helperStarted = true;
    return true;
}

void ExternalCommand::stopHelper()
{
    auto *interface = new org::kde::kpmcore::externalcommand(QStringLiteral("org.kde.kpmcore.externalcommand"),
                    QStringLiteral("/Helper"), QDBusConnection::systemBus());
    QByteArray request;
    const quint64 nonce = interface->getNonce();
    request.setNum(nonce);
    QByteArray hash = QCryptographicHash::hash(request, QCryptographicHash::Sha512);
    interface->exit(privateKey->signMessage(hash, QCA::EMSA3_Raw), nonce);

    delete privateKey;
    delete init;
}

quint64 ExternalCommand::getNonce(QDBusInterface& iface)
{
        QDBusPendingCall pcall = iface.asyncCall(QStringLiteral("getNonce"));
        QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(pcall);
        QEventLoop loop;
        unsigned long long rval = 0;

        auto exitLoop = [&] (QDBusPendingCallWatcher *watcher) {
            loop.exit();

            if (watcher->isError())
                qWarning() << watcher->error();
            else {
                QDBusPendingReply<unsigned long long> reply = *watcher;
                rval = reply;
            }
        };

        connect(watcher, &QDBusPendingCallWatcher::finished, exitLoop);
        loop.exec();
        return rval;
}

void DBusThread::run()
{
    if (!QDBusConnection::systemBus().registerService(QStringLiteral("org.kde.kpmcore.applicationinterface"))) {
        qWarning() << QDBusConnection::systemBus().lastError().message();
        return;
    }
    if (!QDBusConnection::systemBus().registerObject(QStringLiteral("/Application"), this, QDBusConnection::ExportAllSlots)) {
        qWarning() << QDBusConnection::systemBus().lastError().message();
        return;
    }
        
    QEventLoop loop;
    loop.exec();
}
