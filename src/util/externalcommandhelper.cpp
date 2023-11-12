/*
    SPDX-FileCopyrightText: 2017-2022 Andrius Štikonas <andrius@stikonas.eu>
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

#include <filesystem>

#include <fcntl.h>

#include <QtDBus>

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QFileInfo>
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
    if (!QDBusConnection::systemBus().registerObject(QStringLiteral("/Helper"), this, QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals)) {
        exit(-1);
    }

    if (!QDBusConnection::systemBus().registerService(QStringLiteral("org.kde.kpmcore.helperinterface"))) {
        exit(-1);
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
bool ExternalCommandHelper::readData(QFile& device, QByteArray& buffer, const qint64 offset, const qint64 size)
{
    if (!device.isOpen()) {
        if (!device.open(QIODevice::ReadOnly | QIODevice::Unbuffered)) {
            qCritical() << xi18n("Could not open device <filename>%1</filename> for reading.", device.fileName());
            return false;
        }
    }

    // Sequential devices such as /dev/zero or /dev/urandom return false on seek().
    if (!device.isSequential() && !device.seek(offset)) {
        qCritical() << xi18n("Could not seek position %1 on device <filename>%2</filename>.", offset, device.fileName());
        return false;
    }

    buffer = device.read(size);

    if (size != buffer.size()) {
        qCritical() << xi18n("Could not read from device <filename>%1</filename>.", device.fileName());
        return false;
    }

    return true;
}

/** Writes the data from buffer to a given device.
    @param device device or file to write to
    @param buffer the data that we write
    @param offset offset where to begin writing
    @return true on success
*/
bool ExternalCommandHelper::writeData(QFile& device, const QByteArray& buffer, const qint64 offset)
{
    auto flags = QIODevice::WriteOnly | QIODevice::Unbuffered;
    if (!device.isOpen()) {
        if (!device.open(flags)) {
            qCritical() << xi18n("Could not open device <filename>%1</filename> for writing.", device.fileName());
            return false;
        }
    }

    if (!device.seek(offset)) {
        qCritical() << xi18n("Could not seek position %1 on device <filename>%2</filename>.", offset, device.fileName());
        return false;
    }

    if (device.write(buffer) != buffer.size()) {
        qCritical() << xi18n("Could not write to device <filename>%1</filename>.", device.fileName());
        return false;
    }

    return true;
}

/** Creates a new fstab file with given contents.
    @param Contents the data that we write
    @return true on success
*/
bool ExternalCommandHelper::WriteFstab(const QByteArray& fstabContents)
{
    if (!isCallerAuthorized()) {
        return false;
    }
    if (fstabContents.size() > MiB) {
        qCritical() << QStringLiteral("/etc/fstab size limit exceeded.");
        return false;
    }
    QString fstabPath = QStringLiteral("/etc/fstab");
    QFile fstabFile(fstabPath);

    // WriteOnly implies O_TRUNC
    auto flags = QIODevice::WriteOnly | QIODevice::Unbuffered;
    if (!fstabFile.open(flags)) {
        qCritical() << xi18n("Could not open file <filename>%1</filename> for writing.", fstabPath);
        return false;
    }

    if (fstabFile.write(fstabContents) != fstabContents.size()) {
        qCritical() << xi18n("Could not write to file <filename>%1</filename>.", fstabPath);
        return false;
    }

    return true;
}

// If targetDevice is empty then return QByteArray with data that was read from disk.
QVariantMap ExternalCommandHelper::CopyFileData(const QString& sourceDevice, const qint64 sourceOffset, const qint64 sourceLength, const QString& targetDevice, const qint64 targetOffset, const qint64 chunkSize)
{
    if (!isCallerAuthorized()) {
        return {};
    }

    // Avoid division by zero further down
    if (!chunkSize) {
        return {};
    }

    // Prevent some out of memory situations
    if (chunkSize > 100 * MiB) {
        return {};
    }

    // Check for relative paths
    std::filesystem::path sourcePath(sourceDevice.toStdU16String());
    std::filesystem::path targetPath(targetDevice.toStdU16String());
    if(sourcePath.is_relative() || targetPath.is_relative()) {
        return {};
    }

    // Only allow writing to existing files.
    if(!std::filesystem::exists(targetPath)) {
        return {};
    }

    QVariantMap reply;
    reply[QStringLiteral("success")] = true;

    // This enum specified whether individual data chunks are moved left or right
    // When source and target devices are the same we have to be careful not to overwrite
    // source data with newly written data. We don't have to do this if sourceDevice is not
    // targetDevice but there are no disadvantages in applying the same scheme.
    // When partition is moved to the left, we start with the leftmost chunk,
    // and move it further left, then second leftmost chunk and so on.
    // But when we move partition to the right, we start with rightmost chunk.
    // To account for this difference, we introduce CopyDirection variable which takes
    // care of some of the differences in offset calculation between these two cases.
    enum CopyDirection : qint8 {
        Left = 1,
        Right = -1,
    };
    qint8 copyDirection = targetOffset > sourceOffset ? CopyDirection::Right : CopyDirection::Left;

    // Let readOffset (r) and writeOffset (w) be the offsets of the first chunk that we move.
    // When we move data to the left:
    // ______target______         ______source______
    // r                     <-   w=================
    qint64 readOffset = sourceOffset;
    qint64 writeOffset = targetOffset;

    // When we move data to the right, we start moving data from the last chunk
    // ______source______         ______target______
    // =================r    ->                    w
    if (copyDirection == CopyDirection::Right) {
        readOffset = sourceOffset + sourceLength - chunkSize;
        writeOffset = targetOffset + sourceLength - chunkSize;
    }

    const qint64 chunksToCopy = sourceLength / chunkSize;
    const qint64 lastBlock = sourceLength % chunkSize;

    qint64 bytesWritten = 0;
    qint64 chunksCopied = 0;

    QByteArray buffer;
    int percent = 0;
    QElapsedTimer timer;

    timer.start();

    QString reportText = xi18nc("@info:progress", "Copying %1 chunks (%2 bytes) from %3 to %4, direction: %5.", chunksToCopy,
                                              sourceLength, readOffset, writeOffset, copyDirection == CopyDirection::Left ? i18nc("direction: left", "left")
                                              : i18nc("direction: right", "right"));
    Q_EMIT report(reportText);

    bool rval = true;

    QFile target(targetDevice);
    QFile source(sourceDevice);
    while (chunksCopied < chunksToCopy) {
        if (!(rval = readData(source, buffer, readOffset + chunkSize * chunksCopied * copyDirection, chunkSize)))
            break;

        if (!(rval = writeData(target, buffer, writeOffset + chunkSize * chunksCopied * copyDirection)))
            break;

        bytesWritten += buffer.size();

        if (++chunksCopied * 100 / chunksToCopy != percent) {
            percent = chunksCopied * 100 / chunksToCopy;

            if (percent % 5 == 0 && timer.elapsed() > 1000) {
                const qint64 mibsPerSec = (chunksCopied * chunkSize / 1024 / 1024) / (timer.elapsed() / 1000);
                const qint64 estSecsLeft = (100 - percent) * timer.elapsed() / percent / 1000;
                reportText = xi18nc("@info:progress", "Copying %1 MiB/second, estimated time left: %2", mibsPerSec, QTime(0, 0).addSecs(estSecsLeft).toString());
                Q_EMIT report(reportText);
            }
            Q_EMIT progress(percent);
        }
    }

    // copy the remainder
    if (rval && lastBlock > 0) {
        Q_ASSERT(lastBlock < chunkSize);

        const qint64 lastBlockReadOffset = copyDirection == CopyDirection::Left ? readOffset + chunkSize * chunksCopied : sourceOffset;
        const qint64 lastBlockWriteOffset = copyDirection == CopyDirection::Left ? writeOffset + chunkSize * chunksCopied : targetOffset;
        reportText = xi18nc("@info:progress", "Copying remainder of chunk size %1 from %2 to %3.", lastBlock, lastBlockReadOffset, lastBlockWriteOffset);
        Q_EMIT report(reportText);
        rval = readData(source, buffer, lastBlockReadOffset, lastBlock);

        if (rval) {
            rval = writeData(target, buffer, lastBlockWriteOffset);
        }

        if (rval) {
            Q_EMIT progress(100);
            bytesWritten += buffer.size();
        }
    }

    reportText = xi18ncp("@info:progress argument 2 is a string such as 7 bytes (localized accordingly)", "Copying 1 chunk (%2) finished.", "Copying %1 chunks (%2) finished.", chunksCopied, i18np("1 byte", "%1 bytes", bytesWritten));
    Q_EMIT report(reportText);

    reply[QStringLiteral("success")] = rval;
    return reply;
}

QByteArray ExternalCommandHelper::ReadData(const QString& device, const qint64 offset, const qint64 length)
{
    if (!isCallerAuthorized()) {
        return {};
    }

    if (length > MiB) {
        return {};
    }
    if (!std::filesystem::is_block_file(device.toStdU16String())) {
        qWarning() << "Not a block device";
        return {};
    }

    // Do not follow symlinks
    QFileInfo info(device);
    if (info.isSymbolicLink()) {
        qWarning() << "ReadData: device should not be symbolic link";
        return {};
    }
    if (device.left(5) != QStringLiteral("/dev/") || device.left(9) != QStringLiteral("/dev/shm/")) {
        qWarning() << "Error: trying to read data from device not in /dev";
        return {};
    }

    QByteArray buffer;
    QFile sourceDevice;
    int fd = open(device.toLocal8Bit().constData(), O_NOFOLLOW);
    // Negative numbers are error codes
    if (fd < 0) {
        qWarning() << "Error: failed to open device " << device;
        return QByteArray();
    }
    bool rval = sourceDevice.open(fd, QIODevice::ReadOnly | QIODevice::Unbuffered);
    rval = rval && readData(sourceDevice, buffer, offset, length);
    close(fd);
    if (rval) {
        return buffer;
    }
    return QByteArray();
}

bool ExternalCommandHelper::WriteData(const QByteArray& buffer, const QString& targetDevice, const qint64 targetOffset)
{
    if (!isCallerAuthorized()) {
        return false;
    }
    // Do not allow using this helper for writing to arbitrary location
    if ( targetDevice.left(5) != QStringLiteral("/dev/") )
        return false;

    auto targetPath = std::filesystem::path(targetDevice.toStdU16String());
    if (!std::filesystem::is_block_file(targetDevice.toStdU16String())) {
        qWarning() << "Not a block device";
        return {};
    }

    auto canonicalTargetPath = std::filesystem::canonical(targetPath);
    // TODO: Qt6 supports std::filesystem::path
    QFile device(QLatin1String(canonicalTargetPath.c_str()));
    return writeData(device, buffer, targetOffset);
}

QVariantMap ExternalCommandHelper::RunCommand(const QString& command, const QStringList& arguments, const QByteArray& input, const int processChannelMode)
{
    if (!isCallerAuthorized()) {
        return {};
    }

    QVariantMap reply;
    reply[QStringLiteral("success")] = false;

    if (command.isEmpty()) {
        return reply;
    }

    // Compare with command whitelist
    QFileInfo fileInfo(command);
    QString basename = fileInfo.fileName();
    if (allowedCommands.find(basename) == allowedCommands.end()) { // TODO: C++20: replace with contains
        qInfo() << command << "command is not one of the whitelisted commands";
        reply[QStringLiteral("success")] = false;
        return reply;
    }

    // Make sure command is located in the trusted prefix
    QDir prefix = fileInfo.absoluteDir();
    QString dirname = prefix.dirName();
    if (dirname == QStringLiteral("bin") || dirname == QStringLiteral("sbin")) {
        prefix.cdUp();
    }
    if (trustedPrefixes.find(prefix.path()) == trustedPrefixes.end()) { // TODO: C++20: replace with contains
        qInfo() << prefix.path() << "prefix is not one of the trusted command prefixes";
        reply[QStringLiteral("success")] = false;
        return reply;
    }

//  connect(&cmd, &QProcess::readyReadStandardOutput, this, &ExternalCommandHelper::onReadOutput);

    QProcess cmd;
    cmd.setEnvironment( { QStringLiteral("LVM_SUPPRESS_FD_WARNINGS=1") } );

    if((processChannelMode != QProcess::SeparateChannels) && (processChannelMode != QProcess::MergedChannels)) {
        return reply;
    }
    cmd.setProcessChannelMode(static_cast<QProcess::ProcessChannelMode>(processChannelMode));
    cmd.start(command, arguments);
    cmd.write(input);
    cmd.closeWriteChannel();
    cmd.waitForFinished(-1);
    QByteArray output = cmd.readAllStandardOutput();
    reply[QStringLiteral("output")] = output;
    reply[QStringLiteral("exitCode")] = cmd.exitCode();

    reply[QStringLiteral("success")] = true;
    return reply;
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

    // Cache successful authentication requests, so that clients don't need
    // to authenticate multiple times during long partitioning operations.
    // auth_admin_keep is not used intentionally because with current architecture
    // it might lead to data loss if user cancels sfdisk partition boundary adjustment
    // after partition data was moved.
    if (m_serviceWatcher->watchedServices().contains(message().service())) {
        return true;
    }

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
        // track who called into us so we can close when all callers have gone away
        m_serviceWatcher->addWatchedService(message().service());
        return true;
    default:
        sendErrorReply(QDBusError::AccessDenied);
        if (m_serviceWatcher->watchedServices().isEmpty())
            qApp->quit();
        return false;
    }
}

int main(int argc, char ** argv)
{
    QCoreApplication app(argc, argv);
    ExternalCommandHelper helper;
    app.exec();
}

#include "moc_externalcommandhelper.cpp"
