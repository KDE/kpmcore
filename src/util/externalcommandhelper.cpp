/*************************************************************************
 *  Copyright (C) 2017 by Andrius Štikonas <andrius@stikonas.eu>         *
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

#include <QDebug>
#include <QString>

bool ExternalCommandHelper::readData(QByteArray& buffer, qint64 offset, qint64 size)
{
    QStringList arguments = {
                QStringLiteral("skip=") + QString::number(offset),
                QStringLiteral("bs=") + QString::number(size),
                QStringLiteral("count=1"),
                QStringLiteral("iflag=skip_bytes"),
                QStringLiteral("if=") + sourceDevice
                };

    cmd.start(command, arguments);
    cmd.waitForFinished(-1);

    if (cmd.exitCode() == 0) {
        buffer = cmd.readAllStandardOutput();
        return true;
    }
    qDebug() << sourceDevice << " " << offset << " " << size << " cmd exitCode " << cmd.exitCode() << "\n";
    return false;
}

bool ExternalCommandHelper::writeData(QByteArray& buffer, qint64 offset)
{
     QStringList arguments = {
                QStringLiteral("of=") + targetDevice,
                QStringLiteral("seek=") + QString::number(offset),
                QStringLiteral("bs=1M"),
                QStringLiteral("oflag=seek_bytes"),
                QStringLiteral("conv=fsync") };

    cmd.start(command, arguments);
    cmd.write(buffer);
    cmd.closeWriteChannel();
    cmd.waitForFinished(-1);

    if (cmd.exitCode() == 0 ) {
        return true;
    }

    qDebug() << "cmd exitCode "<<cmd.exitCode() << "\n";

    return false;
}

ActionReply ExternalCommandHelper::copyblockshelper(const QVariantMap& args)
{
    qDebug() << "ExternalCommandHelper::copyBlocksHelper\n";

    command = args[QStringLiteral("command")].toString();

    qint64 blockSize = args[QStringLiteral("blockSize")].toInt();
    qint64 blocksToCopy = args[QStringLiteral("blocksToCopy")].toInt();
    qint64 readOffset = args[QStringLiteral("readOffset")].toInt();
    qint64 writeOffset = args[QStringLiteral("writeOffset")].toInt();
    qint32 copyDirection = args[QStringLiteral("copyDirection")].toInt();
    sourceDevice = args[QStringLiteral("sourceDevice")].toString();
    targetDevice = args[QStringLiteral("targetDevice")].toString();
    qint64 lastBlock = args[QStringLiteral("lastBlock")].toInt();
    qint64 sourceFirstByte = args[QStringLiteral("sourceFirstByte")].toInt();
    qint64 targetFirstByte = args[QStringLiteral("targetFirstByte")].toInt();

    QStringList environment = args[QStringLiteral("environment")].toStringList();

    //qDebug() << command<< " " << sourceDevice <<" " << targetDevice << " blocksToCopy: " << blocksToCopy << " blockSize" << blockSize << " " << readOffset<<" "<<writeOffset<<" "<<environment<<"\n";

    ActionReply reply;

    //connect(&cmd, &QProcess::readyReadStandardOutput, this, &ExternalCommandHelper::onReadOutput);

    QByteArray buffer;

    cmd.setEnvironment(environment);

    qint64 blocksCopied = 0;

    //QByteArray buffer;
    int percent = 0;
    //QTime t;
    bool rval = true;
    //qDebug() << command <<"\n";
    while (blocksCopied < blocksToCopy) {
        if (!(rval = readData(buffer, readOffset + blockSize * blocksCopied * copyDirection, blockSize)))
            break;

        if (!(rval = writeData(buffer, writeOffset + blockSize * blocksCopied * copyDirection)))
            break;
        //qDebug() << "Exit code" <<rval <<"\n";

        if (++blocksCopied * 100 / blocksToCopy != percent) {
            percent = blocksCopied * 100 / blocksToCopy;

            //if (percent % 5 == 0 && t.elapsed() > 1000) {
              //  const qint64 mibsPerSec = (blocksCopied * blockSize / 1024 / 1024) / (t.elapsed() / 1000);
               // const qint64 estSecsLeft = (100 - percent) * t.elapsed() / percent / 1000;
               // report.line() << xi18nc("@info:progress", "Copying %1 MiB/second, estimated time left: %2", mibsPerSec, QTime(0, 0).addSecs(estSecsLeft).toString());
            //}
            HelperSupport::progressStep(percent);
        }
    }

    //const qint64 lastBlock = args[QStringLiteral("lastBlock")];

    // copy the remainder
    if (rval && lastBlock > 0) {
        Q_ASSERT(lastBlock < blockSize);

        const qint64 lastBlockReadOffset = copyDirection > 0 ? readOffset + blockSize * blocksCopied : sourceFirstByte;
        const qint64 lastBlockWriteOffset = copyDirection > 0 ? writeOffset + blockSize * blocksCopied : targetFirstByte;
        //report.line() << xi18nc("@info:progress", "Copying remainder of block size %1 from %2 to %3.", lastBlock, lastBlockReadOffset, lastBlockWriteOffset);

        rval = readData(buffer, lastBlockReadOffset, lastBlock);

        if (rval)
            rval = writeData(buffer, lastBlockWriteOffset);

        if (rval){
            qDebug() << "Percent 100"<<"\n";
            emit progress(100);
        }

    }

    //report.line() << xi18ncp("@info:progress argument 2 is a string such as 7 bytes (localized accordingly)", "Copying 1 block (%2) finished.", "Copying %1 blocks (%2) finished.", blocksCopied, i18np("1 byte", "%1 bytes", target.bytesWritten()));

    return reply;
}

ActionReply ExternalCommandHelper::start(const QVariantMap& args)
{
    ActionReply reply;
    QString command = args[QStringLiteral("command")].toString();
    QStringList arguments = args[QStringLiteral("arguments")].toStringList();
    QStringList environment = args[QStringLiteral("environment")].toStringList();
    QByteArray input = args[QStringLiteral("input")].toByteArray();

//     connect(&cmd, &QProcess::readyReadStandardOutput, this, &ExternalCommandHelper::onReadOutput);

    cmd.setEnvironment(environment);
    cmd.start(command, arguments);
    cmd.write(input);
    cmd.closeWriteChannel();
    cmd.waitForFinished(-1);
    QByteArray output = cmd.readAllStandardOutput();
    reply.addData(QStringLiteral("output"), output);
    reply.addData(QStringLiteral("exitCode"), cmd.exitCode());

    return reply;
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
