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
#include "externalcommand_interface.h"
#include "externalcommand_whitelist.h"

#include <QtDBus>
#include <QDebug>
#include <QFile>
#include <QString>
#include <QTime>
#include <QVariant>

#include <KLocalizedString>

/** Initialize ExternalCommandHelper Daemon and prepare DBus interface
 *
 * KAuth helper runs in the background until application exits.
 * To avoid forever running helper in case of application crash
 * ExternalCommand class opens a DBus service that we monitor for changes.
 * If helper is not busy then it exits when the client services gets
 * unregistered. Otherwise,
 * we wait for the current job to finish before exiting, so even in case
 * of main application crash, we do not leave partially moved data.
 *
 * This helper also starts another DBus interface where it listens to
 * command execution requests from the application that started the helper.
 * These requests are validated using public key cryptography, to prevent
 * other unprivileged applications from gaining root privileges.
*/
ActionReply ExternalCommandHelper::init(const QVariantMap& args)
{
    ActionReply reply;
    if (!QDBusConnection::systemBus().isConnected()) {
        qWarning() << "Could not connect to DBus system bus";
        reply.addData(QStringLiteral("success"), false);
        return reply;
    }

    if (!QDBusConnection::systemBus().registerService(QStringLiteral("org.kde.kpmcore.helperinterface"))) {
        qWarning() << QDBusConnection::systemBus().lastError().message();
        reply.addData(QStringLiteral("success"), false);
        return reply;
    }
    if (!QDBusConnection::systemBus().registerObject(QStringLiteral("/Helper"), this, QDBusConnection::ExportAllSlots)) {
        qWarning() << QDBusConnection::systemBus().lastError().message();
        reply.addData(QStringLiteral("success"), false);
        return reply;
    }

    m_publicKey = QCA::PublicKey::fromDER(args[QStringLiteral("pubkey")].toByteArray());

    m_loop = std::make_unique<QEventLoop>();
    HelperSupport::progressStep(QVariantMap());

    // End the loop and return only once the client is done using us.
    auto serviceWatcher =
            new QDBusServiceWatcher(QStringLiteral("org.kde.kpmcore.applicationinterface"),
                                    QDBusConnection::systemBus(),
                                    QDBusServiceWatcher::WatchForUnregistration,
                                    this);
    connect(serviceWatcher, &QDBusServiceWatcher::serviceUnregistered,
            [this]() {
        m_loop->exit();
    });

    m_loop->exec();
    reply.addData(QStringLiteral("success"), true);

    return reply;
}

/** Generates cryptographic nonce
 *  @return nonce
*/
quint64 ExternalCommandHelper::getNonce()
{
    quint64 nonce = m_Generator.generate();
    m_Nonces.insert(nonce);
    return nonce;
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
        qCritical() << xi18n("Could not seek position %1 on device <filename>%2</filename>.", offset, targetDevice);
        return false;
    }

    if (device.write(buffer) != buffer.size()) {
        qCritical() << xi18n("Could not write to device <filename>%1</filename>.", targetDevice);
        return false;
    }
    return true;
}

QVariantMap ExternalCommandHelper::copyblocks(const QByteArray& signature, const quint64 nonce, const QString& sourceDevice, const qint64 sourceFirstByte, const qint64 sourceLength, const QString& targetDevice, const qint64 targetFirstByte, const qint64 blockSize)
{
    QVariantMap reply;
    reply[QStringLiteral("success")] = true;

    if (m_Nonces.find(nonce) != m_Nonces.end())
        m_Nonces.erase( nonce );
    else {
        reply[QStringLiteral("success")] = false;
        return reply;
    }

    QByteArray request;

    request.setNum(nonce);
    request.append(sourceDevice.toUtf8());
    request.append(QByteArray::number(sourceFirstByte));
    request.append(QByteArray::number(sourceLength));
    request.append(targetDevice.toUtf8());
    request.append(QByteArray::number(targetFirstByte));
    request.append(QByteArray::number(blockSize));

    QByteArray hash = QCryptographicHash::hash(request, QCryptographicHash::Sha512);
    if (!m_publicKey.verifyMessage(hash, signature, QCA::EMSA3_Raw)) {
        qCritical() << xi18n("Invalid cryptographic signature");
        reply[QStringLiteral("success")] = false;
        return reply;
    }

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

        if (rval) {
            if (targetDevice.isEmpty())
                reply[QStringLiteral("targetByteArray")] = buffer;
            else
                rval = writeData(targetDevice, buffer, lastBlockWriteOffset);
        }

        if (rval) {
            HelperSupport::progressStep(100);
            bytesWritten += buffer.size();
        }
    }

    report[QStringLiteral("report")] = xi18ncp("@info:progress argument 2 is a string such as 7 bytes (localized accordingly)", "Copying 1 block (%2) finished.", "Copying %1 blocks (%2) finished.", blocksCopied, i18np("1 byte", "%1 bytes", bytesWritten));
    HelperSupport::progressStep(report);

    reply[QStringLiteral("success")] = rval;
    return reply;
}

QVariantMap ExternalCommandHelper::start(const QByteArray& signature, const quint64 nonce, const QString& command, const QStringList& arguments, const QByteArray& input, const int processChannelMode)
{
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
    QVariantMap reply;
    reply[QStringLiteral("success")] = true;

    if (m_Nonces.find(nonce) != m_Nonces.end())
        m_Nonces.erase( nonce );
    else {
        reply[QStringLiteral("success")] = false;
        return reply;
    }

    if (command.isEmpty()) {
        reply[QStringLiteral("success")] = false;
        return reply;
    }

    QByteArray request;
    request.setNum(nonce);
    request.append(command.toUtf8());
    for (const auto &argument : arguments)
        request.append(argument.toUtf8());
    request.append(input);
    request.append(processChannelMode);
    QByteArray hash = QCryptographicHash::hash(request, QCryptographicHash::Sha512);
    if (!m_publicKey.verifyMessage(hash, signature, QCA::EMSA3_Raw)) {
        qCritical() << xi18n("Invalid cryptographic signature");
        reply[QStringLiteral("success")] = false;
        return reply;
    }

    // Compare with command whitelist
    QString basename = command.mid(command.lastIndexOf(QLatin1Char('/')) + 1);
    if (std::find(std::begin(allowedCommands), std::end(allowedCommands), basename) == std::end(allowedCommands)) {
        // TODO: notify the user
        m_loop->exit();
        reply[QStringLiteral("success")] = false;
        return reply;
    }

//     connect(&cmd, &QProcess::readyReadStandardOutput, this, &ExternalCommandHelper::onReadOutput);

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

void ExternalCommandHelper::exit(const QByteArray& signature, const quint64 nonce)
{
    QByteArray request;
    if (m_Nonces.find(nonce) == m_Nonces.end())
        return;

    request.setNum(nonce);
    QByteArray hash = QCryptographicHash::hash(request, QCryptographicHash::Sha512);
    if (!m_publicKey.verifyMessage(hash, signature, QCA::EMSA3_Raw)) {
        qCritical() << xi18n("Invalid cryptographic signature");
        return;
    }

    m_loop->exit();

    QDBusConnection::systemBus().unregisterObject(QStringLiteral("/Helper"));
    QDBusConnection::systemBus().unregisterService(QStringLiteral("org.kde.kpmcore.helperinterface"));
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
