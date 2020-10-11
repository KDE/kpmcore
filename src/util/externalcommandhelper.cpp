/*
    SPDX-FileCopyrightText: 2017-2020 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2018 Huzaifa Faruqui <huzaifafaruqui@gmail.com>
    SPDX-FileCopyrightText: 2018 Caio Jordão Carvalho <caiojcarvalho@gmail.com>
    SPDX-FileCopyrightText: 2018-2019 Harald Sitter <sitter@kde.org>
    SPDX-FileCopyrightText: 2018 Simon Depiets <sdepiets@gmail.com>
    SPDX-FileCopyrightText: 2019 Shubham Jangra <aryan100jangid@gmail.com>
    SPDX-FileCopyrightText: 2020 David Edmundson <kde@davidedmundson.co.uk>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "externalcommandhelper.h"
#include "externalcommand_whitelist.h"

#include <QtDBus>

#include <QCoreApplication>
#include <QDebug>
#include <QElapsedTimer>
#include <QFile>
#include <QString>
#include <QVariant>

#include <KLocalizedString>
#include <PolkitQt1/Authority>
#include <PolkitQt1/Subject>

#include <polkitqt1-version.h>

/** Initialize ExternalCommandHelper Daemon and prepare DBus interface
 *
 * This helper runs in the background until all applications using it exit.
 * If helper is not busy then it exits when the client services gets
 * unregistered. In case the client crashes, the helper waits
 * for the current job to finish before exiting, to avoid leaving partially moved data.
 *
 * This helper starts DBus interface where it listens to command execution requests.
 * New clients connecting to the helper have to authenticate using Polkit.
*/

ExternalCommandHelper::ExternalCommandHelper()
{
    if (!QDBusConnection::systemBus().registerObject(QStringLiteral("/Helper"), this, QDBusConnection::ExportAllSlots)) {
        ::exit(-1);
    }

    if (!QDBusConnection::systemBus().registerService(QStringLiteral("org.kde.kpmcore.helperinterface"))) {
        ::exit(-1);
    }

    // we know this service must be registered already as DBus policy blocks calls from anyone else
    m_serviceWatcher = new QDBusServiceWatcher(this);
    m_serviceWatcher->setConnection(QDBusConnection ::systemBus());
    m_serviceWatcher->setWatchMode(QDBusServiceWatcher::WatchForUnregistration);

    connect(m_serviceWatcher, &QDBusServiceWatcher::serviceUnregistered, qApp, [this](const QString &service) {
        m_serviceWatcher->removeWatchedService(service);
        if (m_serviceWatcher->watchedServices().isEmpty()) {
            qApp->quit();
        }
    });
}

/** Reads the given number of bytes from the sourceDevice into the given buffer.
    @param sourceDevice device or file to read from
    @param buffer buffer to store the bytes read in
    @param offset offset where to begin reading
    @param size the number of bytes to read
    @return true on success
*/
bool ExternalCommandHelper::readData(const QString& sourceDevice, QByteArray& buffer, const qint64 offset, const qint64 size)
{
    QFile device(sourceDevice);

    if (!device.open(QIODevice::ReadOnly | QIODevice::Unbuffered)) {
        qCritical() << xi18n("Could not open device <filename>%1</filename> for reading.", sourceDevice);
        return false;
    }

    if (!device.seek(offset)) {
        qCritical() << xi18n("Could not seek position %1 on device <filename>%2</filename>.", offset, sourceDevice);
        return false;
    }

    buffer = device.read(size);

    if (size != buffer.size()) {
        qCritical() << xi18n("Could not read from device <filename>%1</filename>.", sourceDevice);
         return false;
    }

    return true;
}

/** Writes the data from buffer to a given device.
    @param targetDevice device or file to write to
    @param buffer the data that we write
    @param offset offset where to begin writing
    @return true on success
*/
bool ExternalCommandHelper::writeData(const QString &targetDevice, const QByteArray& buffer, const qint64 offset)
{
    QFile device(targetDevice);

    auto flags = QIODevice::WriteOnly | QIODevice::Unbuffered | QIODevice::Append;
    if (!device.open(flags)) {
        qCritical() << xi18n("Could not open device <filename>%1</filename> for writing.", targetDevice);
        return false;
    }

    if (!device.seek(offset)) {
        qCritical() << xi18n("Could not seek position %1 on device <filename>%2</filename>.", offset, targetDevice);
        return false;
    }

    if (device.write(buffer) != buffer.size()) {
        qCritical() << xi18n("Could not write to device <filename>%1</filename>.", targetDevice);
        return false;
    }

    return true;
}

/** Creates a new file with given contents.
    @param filePath file to write to
    @param fileContents the data that we write
    @return true on success
*/
bool ExternalCommandHelper::createFile(const QString &filePath, const QByteArray& fileContents)
{
    if (!isCallerAuthorized()) {
        return false;
    }
    QFile device(filePath);

    auto flags = QIODevice::WriteOnly | QIODevice::Unbuffered;
    if (!device.open(flags)) {
        qCritical() << xi18n("Could not open file <filename>%1</filename> for writing.", filePath);
        return false;
    }

    if (device.write(fileContents) != fileContents.size()) {
        qCritical() << xi18n("Could not write to file <filename>%1</filename>.", filePath);
        return false;
    }

    return true;
}

// If targetDevice is empty then return QByteArray with data that was read from disk.
QVariantMap ExternalCommandHelper::copyblocks(const QString& sourceDevice, const qint64 sourceFirstByte, const qint64 sourceLength, const QString& targetDevice, const qint64 targetFirstByte, const qint64 blockSize)
{
    if (!isCallerAuthorized()) {
        return QVariantMap();
    }
    QVariantMap reply;
    reply[QStringLiteral("success")] = true;

    const qint64 blocksToCopy = sourceLength / blockSize;
    qint64 readOffset = sourceFirstByte;
    qint64 writeOffset = targetFirstByte;
    qint32 copyDirection = 1;

    if (targetFirstByte > sourceFirstByte) {
        readOffset = sourceFirstByte + sourceLength - blockSize;
        writeOffset = targetFirstByte + sourceLength - blockSize;
        copyDirection = -1;
    }

    const qint64 lastBlock = sourceLength % blockSize;

    qint64 bytesWritten = 0;
    qint64 blocksCopied = 0;

    QByteArray buffer;
    int percent = 0;
    QElapsedTimer timer;

    timer.start();

    QVariantMap report;

    report[QStringLiteral("report")] = xi18nc("@info:progress", "Copying %1 blocks (%2 bytes) from %3 to %4, direction: %5.", blocksToCopy,
                                              sourceLength, readOffset, writeOffset, copyDirection == 1 ? i18nc("direction: left", "left")
                                              : i18nc("direction: right", "right"));

    //HelperSupport::progressStep(report);

    bool rval = true;

    while (blocksCopied < blocksToCopy && !targetDevice.isEmpty()) {
        if (!(rval = readData(sourceDevice, buffer, readOffset + blockSize * blocksCopied * copyDirection, blockSize)))
            break;

        if (!(rval = writeData(targetDevice, buffer, writeOffset + blockSize * blocksCopied * copyDirection)))
            break;

        bytesWritten += buffer.size();

        if (++blocksCopied * 100 / blocksToCopy != percent) {
            percent = blocksCopied * 100 / blocksToCopy;

            if (percent % 5 == 0 && timer.elapsed() > 1000) {
                const qint64 mibsPerSec = (blocksCopied * blockSize / 1024 / 1024) / (timer.elapsed() / 1000);
                const qint64 estSecsLeft = (100 - percent) * timer.elapsed() / percent / 1000;
                report[QStringLiteral("report")]=  xi18nc("@info:progress", "Copying %1 MiB/second, estimated time left: %2", mibsPerSec, QTime(0, 0).addSecs(estSecsLeft).toString());
                //HelperSupport::progressStep(report);
            }
            //HelperSupport::progressStep(percent);
        }
    }

    // copy the remainder
    if (rval && lastBlock > 0) {
        Q_ASSERT(lastBlock < blockSize);

        const qint64 lastBlockReadOffset = copyDirection > 0 ? readOffset + blockSize * blocksCopied : sourceFirstByte;
        const qint64 lastBlockWriteOffset = copyDirection > 0 ? writeOffset + blockSize * blocksCopied : targetFirstByte;
        report[QStringLiteral("report")]= xi18nc("@info:progress", "Copying remainder of block size %1 from %2 to %3.", lastBlock, lastBlockReadOffset, lastBlockWriteOffset);
        //HelperSupport::progressStep(report);
        rval = readData(sourceDevice, buffer, lastBlockReadOffset, lastBlock);

        if (rval) {
            if (targetDevice.isEmpty())
                reply[QStringLiteral("targetByteArray")] = buffer;
            else
                rval = writeData(targetDevice, buffer, lastBlockWriteOffset);
        }

        if (rval) {
	    //HelperSupport::progressStep(100);
            bytesWritten += buffer.size();
        }
    }

    report[QStringLiteral("report")] = xi18ncp("@info:progress argument 2 is a string such as 7 bytes (localized accordingly)", "Copying 1 block (%2) finished.", "Copying %1 blocks (%2) finished.", blocksCopied, i18np("1 byte", "%1 bytes", bytesWritten));
    //HelperSupport::progressStep(report);

    reply[QStringLiteral("success")] = rval;
    return reply;
}

bool ExternalCommandHelper::writeData(const QByteArray& buffer, const QString& targetDevice, const qint64 targetFirstByte)
{
    if (!isCallerAuthorized()) {
        return false;
    }
    // Do not allow using this helper for writing to arbitrary location
    if ( targetDevice.left(5) != QStringLiteral("/dev/") )
        return false;

    return writeData(targetDevice, buffer, targetFirstByte);
}

bool ExternalCommandHelper::createFile(const QByteArray& fileContents, const QString& filePath)
{
    if (!isCallerAuthorized()) {
        return false;
    }
    // Do not allow using this helper for writing to arbitrary location
    if ( !filePath.contains(QStringLiteral("/etc/fstab")) )
        return false;

    return createFile(filePath, fileContents);
}

QVariantMap ExternalCommandHelper::start(const QString& command, const QStringList& arguments, const QByteArray& input, const int processChannelMode)
{
    if (!isCallerAuthorized()) {
        return QVariantMap();
    }
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
    QVariantMap reply;
    reply[QStringLiteral("success")] = true;

    if (command.isEmpty()) {
        reply[QStringLiteral("success")] = false;
        return reply;
    }

    // Compare with command whitelist
    QString basename = command.mid(command.lastIndexOf(QLatin1Char('/')) + 1);
    if (std::find(std::begin(allowedCommands), std::end(allowedCommands), basename) == std::end(allowedCommands)) {
        qInfo() << command <<" command is not one of the whitelisted command";
        qApp->quit();
        reply[QStringLiteral("success")] = false;
        return reply;
    }

//  connect(&cmd, &QProcess::readyReadStandardOutput, this, &ExternalCommandHelper::onReadOutput);

    m_cmd.setEnvironment( { QStringLiteral("LVM_SUPPRESS_FD_WARNINGS=1") } );
    m_cmd.setProcessChannelMode(static_cast<QProcess::ProcessChannelMode>(processChannelMode));
    m_cmd.start(command, arguments);
    m_cmd.write(input);
    m_cmd.closeWriteChannel();
    m_cmd.waitForFinished(-1);
    QByteArray output = m_cmd.readAllStandardOutput();
    reply[QStringLiteral("output")] = output;
    reply[QStringLiteral("exitCode")] = m_cmd.exitCode();

    return reply;
}

void ExternalCommandHelper::exit()
{
    if (!isCallerAuthorized()) {
        return;
    }
    qApp->quit();
}

void ExternalCommandHelper::onReadOutput()
{
/*    const QByteArray s = cmd.readAllStandardOutput();

      if(output.length() > 10*1024*1024) { // prevent memory overflow for badly corrupted file systems
        if (report())
            report()->line() << xi18nc("@info:status", "(Command is printing too much output)");
            return;
     }

     output += s;

     if (report())
         *report() << QString::fromLocal8Bit(s);*/
}

bool ExternalCommandHelper::isCallerAuthorized()
{
    if (!calledFromDBus()) {
        return false;
    }

    // track who called into us so we can close when all callers have gone away
    // this has to happen before authorisation as anyone could have activated us
    m_serviceWatcher->addWatchedService(message().service());

    PolkitQt1::SystemBusNameSubject subject(message().service());
    PolkitQt1::Authority *authority = PolkitQt1::Authority::instance();

    PolkitQt1::Authority::Result result;
    QEventLoop e;
    connect(authority, &PolkitQt1::Authority::checkAuthorizationFinished, &e, [&e, &result](PolkitQt1::Authority::Result _result) {
        result = _result;
        e.quit();
    });

    authority->checkAuthorization(QStringLiteral("org.kde.kpmcore.externalcommand.init"), subject, PolkitQt1::Authority::AllowUserInteraction);
    e.exec();

    if (authority->hasError()) {
        qDebug() << "Encountered error while checking authorization, error code:" << authority->lastError() << authority->errorDetails();
        authority->clearError();
    }

    switch (result) {
    case PolkitQt1::Authority::Yes:
        return true;
    default:
        sendErrorReply(QDBusError::AccessDenied);
        return false;
    }
}

int main(int argc, char ** argv)
{
    QCoreApplication app(argc, argv);
    ExternalCommandHelper helper;
    app.exec();
}
