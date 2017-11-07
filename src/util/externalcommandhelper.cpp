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
