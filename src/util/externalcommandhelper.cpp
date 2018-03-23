/*************************************************************************
 *  Copyright (C) 2017-2018 by Andrius Štikonas <andrius@stikonas.eu>    *
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

#include "externalcommandhelper.h"

#include <QDate>
#include <QtDBus>
#include <QDBusContext>
#include <QDebug>
#include <QFile>
#include <QString>
#include <QTime>
#include <QVariant>

#include <KLocalizedString>

/** Initialize ExternalCommandHelper Daemon and prepare DBus interface
*/
ActionReply ExternalCommandHelper::init(const QVariantMap& args)
{
    ActionReply reply;
    if (!QDBusConnection::systemBus().isConnected()) {
        qWarning() << "Could not connect to DBus session bus";
        reply.addData(QStringLiteral("success"), false);
        return reply;
    }
    m_callerUuid = args[QStringLiteral("callerUuid")].toString();

    if (!QDBusConnection::systemBus().registerService(QStringLiteral("org.kde.kpmcore.helperinterface"))) {
        qWarning() << QDBusConnection::systemBus().lastError().message();
        reply.addData(QStringLiteral("success"), false);
        return reply;
    }
    QDBusConnection::systemBus().registerObject(QStringLiteral("/Helper"), this, QDBusConnection::ExportAllSlots);

    m_pingTime = new QDateTime(QDateTime::currentDateTime());

    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &ExternalCommandHelper::checkPing);
    timer->start(20000); // check ping every 20 secs

    HelperSupport::progressStep(QVariantMap());
    m_loop.exec();
    reply.addData(QStringLiteral("success"), true);

    return reply;
}

/** Reads the given number of bytes from the sourceDevice into the given buffer.
    @param sourceDevice device or file to read from
    @param buffer buffer to store the bytes read in
    @param offset offset where to begin reading
    @param size the number of bytes to read
    @return true on success
*/
bool ExternalCommandHelper::readData(const QString& sourceDevice, QByteArray& buffer, qint64 offset, qint64 size)
{
    QFile device(sourceDevice);

    if (!device.open(QIODevice::ReadOnly | QIODevice::Unbuffered)) {
        qCritical() << xi18n("Could not open device <filename>%1</filename> for reading.", sourceDevice);
        return false;
    }

    if (!device.seek(offset)) {
        qCritical() << xi18n("Could not seek position %1 on device <filename>%1</filename>.", sourceDevice);
        return false;
    }

    buffer = device.read(size);

    if (size != buffer.size()) {
        qCritical() << xi18n("Could not read from device <filename>%1</filename>.", sourceDevice);
         return false;
    }

    return true;
}

/** Writes the data from buffer to a given device or file.
    @param targetDevice device or file to write to
    @param buffer the data that we write
    @param offset offset where to begin writing
    @return true on success
*/
bool ExternalCommandHelper::writeData(const QString &targetDevice, const QByteArray& buffer, qint64 offset)
{
    QFile device(targetDevice);
    if (!device.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Unbuffered)) {
        qCritical() << xi18n("Could not open device <filename>%1</filename> for writing.", targetDevice);
        return false;
    }

    if (!device.seek(offset)) {
        qCritical() << xi18n("Could not seek position %1 on device <filename>%1</filename>.", targetDevice);
        return false;
    }

    if (device.write(buffer) != buffer.size()) {
        qCritical() << xi18n("Could not write to device <filename>%1</filename>.", targetDevice);
        return false;
    }
    return true;
}

bool ExternalCommandHelper::copyblocks(const QString& Uuid, const QString& sourceDevice, const qint64 sourceFirstByte, const qint64 sourceLength, const QString& targetDevice, const qint64 targetFirstByte, const qint64 blockSize)
{
    if (!isCallerAuthorized(Uuid))
        return false;

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
    QTime t;

    t.start();

    QVariantMap report;

    report[QStringLiteral("report")] = xi18nc("@info:progress", "Copying %1 blocks (%2 bytes) from %3 to %4, direction: %5.", blocksToCopy,
                                              sourceLength, readOffset, writeOffset, copyDirection == 1 ? i18nc("direction: left", "left")
                                              : i18nc("direction: right", "right"));

    HelperSupport::progressStep(report);

    bool rval = true;

    while (blocksCopied < blocksToCopy) {
        if (!(rval = readData(sourceDevice, buffer, readOffset + blockSize * blocksCopied * copyDirection, blockSize)))
            break;

        if (!(rval = writeData(targetDevice, buffer, writeOffset + blockSize * blocksCopied * copyDirection)))
            break;

        bytesWritten += buffer.size();

        if (++blocksCopied * 100 / blocksToCopy != percent) {
            percent = blocksCopied * 100 / blocksToCopy;

            if (percent % 5 == 0 && t.elapsed() > 1000) {
                const qint64 mibsPerSec = (blocksCopied * blockSize / 1024 / 1024) / (t.elapsed() / 1000);
                const qint64 estSecsLeft = (100 - percent) * t.elapsed() / percent / 1000;
                report[QStringLiteral("report")]=  xi18nc("@info:progress", "Copying %1 MiB/second, estimated time left: %2", mibsPerSec, QTime(0, 0).addSecs(estSecsLeft).toString());
                HelperSupport::progressStep(report);
            }
            HelperSupport::progressStep(percent);
        }
    }

    // copy the remainder
    if (rval && lastBlock > 0) {
        Q_ASSERT(lastBlock < blockSize);

        const qint64 lastBlockReadOffset = copyDirection > 0 ? readOffset + blockSize * blocksCopied : sourceFirstByte;
        const qint64 lastBlockWriteOffset = copyDirection > 0 ? writeOffset + blockSize * blocksCopied : targetFirstByte;
        report[QStringLiteral("report")]= xi18nc("@info:progress", "Copying remainder of block size %1 from %2 to %3.", lastBlock, lastBlockReadOffset, lastBlockWriteOffset);
        HelperSupport::progressStep(report);
        rval = readData(sourceDevice, buffer, lastBlockReadOffset, lastBlock);

        if (rval)
            rval = writeData(targetDevice, buffer, lastBlockWriteOffset);

        if (rval) {
            HelperSupport::progressStep(100);
            bytesWritten += buffer.size();
        }
    }

    report[QStringLiteral("report")] = xi18ncp("@info:progress argument 2 is a string such as 7 bytes (localized accordingly)", "Copying 1 block (%2) finished.", "Copying %1 blocks (%2) finished.", blocksCopied, i18np("1 byte", "%1 bytes", bytesWritten));
    HelperSupport::progressStep(report);

    return rval;
}

QVariantMap ExternalCommandHelper::start(const QString& Uuid, const QString& command, const QStringList& arguments, const QByteArray& input, const QStringList& environment)
{
    QVariantMap reply;
    if (!isCallerAuthorized(Uuid)) {
        reply[QStringLiteral("success")] = false;
        return reply;
    }

//     connect(&cmd, &QProcess::readyReadStandardOutput, this, &ExternalCommandHelper::onReadOutput);

    m_cmd.setEnvironment(environment);
    m_cmd.start(command, arguments);
    m_cmd.write(input);
    m_cmd.closeWriteChannel();
    m_cmd.waitForFinished(-1);
    QByteArray output = m_cmd.readAllStandardOutput();
    reply[QStringLiteral("output")] = output;
    reply[QStringLiteral("exitCode")] = m_cmd.exitCode();

    return reply;
}

bool ExternalCommandHelper::isCallerAuthorized(const QString& Uuid)
{
    if (Uuid != m_callerUuid) {
        qWarning() << "Caller is not authorized";
        return false;
    }

    return true;
}

void ExternalCommandHelper::exit(const QString& Uuid)
{
    if (!isCallerAuthorized(Uuid))
        return;
    m_loop.exit();

    if (QDBusConnection::systemBus().unregisterService(QStringLiteral("org.kde.kpmcore.helperinterface")))
        qDebug() << "org.kde.kpmcore.helperinterface unregistered";

    QDBusConnection::systemBus().unregisterObject(QStringLiteral("/Helper"));
}

void ExternalCommandHelper::ping(const QString &Uuid)
{
    if (!isCallerAuthorized(Uuid))
        return;

    // update ping
    m_pingTime->setDate(QDate::currentDate());
    m_pingTime->setTime(QTime::currentTime());
}

void ExternalCommandHelper::checkPing()
{
    qint64 mSecsSinceLastPing = m_pingTime->msecsTo(QDateTime::currentDateTime());

    qDebug() << mSecsSinceLastPing / 1000.0 << " seconds since the last ping.";

    if (mSecsSinceLastPing >= 42000) { // more than 42 seconds since the last ping
        exit(m_callerUuid);
    }
}

void ExternalCommandHelper::onReadOutput()
{
//     const QByteArray s = cmd.readAllStandardOutput();

//     if(output.length() > 10*1024*1024) { // prevent memory overflow for badly corrupted file systems
//         if (report())
//             report()->line() << xi18nc("@info:status", "(Command is printing too much output)");
//         return;
//     }

//     output += s;

//     if (report())
//         *report() << QString::fromLocal8Bit(s);
}

KAUTH_HELPER_MAIN("org.kde.kpmcore.externalcommand", ExternalCommandHelper)
