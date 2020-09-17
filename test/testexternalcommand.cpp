/*
    SPDX-FileCopyrightText: 2018 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2019 Shubham Jangra <aryan100jangid@gmail.com>
    SPDX-FileCopyrightText: 2019 Albert Astals Cid <aacid@kde.org>

    SPDX-License-Identifier: GPL-3.0-or-later
*/


#include "helpers.h"
#include "backend/corebackendmanager.h"
#include "util/externalcommand.h"

#include <QCoreApplication>
#include <QDebug>
#include <QThread>

class runcmd : public QThread
{
    public:
    void run() override
    {
        ExternalCommand blkidCmd(QStringLiteral("blkid"), {});

        // ExternalCommadHelper will refuse to run this or any other command which is not whitelisted.
        // See src/util/externalcommand_whitelist.h for whitelisted commands.
        blkidCmd.run();
        qDebug().noquote() << blkidCmd.output();
    }
};

class runcmd2 : public QThread
{
    public:
    void run() override
    {
        ExternalCommand lsblkCmd(QStringLiteral("lsblk"), { QStringLiteral("--nodeps"), QStringLiteral("--json") });
        lsblkCmd.run();
        qDebug().noquote() << lsblkCmd.output();
    }
};


int main( int argc, char **argv )
{
    QCoreApplication app(argc, argv);
    KPMCoreInitializer i(QStringLiteral("pmsfdiskbackendplugin"));

    runcmd a;
    runcmd2 b;

    a.start();
    a.wait();

    b.start();
    b.wait();

    return 0;
}
