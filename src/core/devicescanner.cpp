/*
    SPDX-FileCopyrightText: 2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2014-2020 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2016 Chantara Tith <tith.chantara@gmail.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "core/devicescanner.h"

#include "backend/corebackend.h"
#include "backend/corebackendmanager.h"

#include "core/operationstack.h"
#include "core/device.h"
#include "core/diskdevice.h"

#include "fs/lvm2_pv.h"

#include "util/externalcommand.h"

#include <QRegularExpression>

/** Constructs a DeviceScanner
    @param ostack the OperationStack where the devices will be created
*/
DeviceScanner::DeviceScanner(QObject* parent, OperationStack& ostack) :
    QThread(parent),
    m_OperationStack(ostack)
{
    setupConnections();
}

void DeviceScanner::setupConnections()
{
    connect(CoreBackendManager::self()->backend(), &CoreBackend::scanProgress, this, &DeviceScanner::progress);
}

void DeviceScanner::clear()
{
    operationStack().clearOperations();
    operationStack().clearDevices();
}

void DeviceScanner::run()
{
    scan();
}

void DeviceScanner::scan()
{
    Q_EMIT progress(QString(), 0);

    clear();

    const QList<Device*> deviceList = CoreBackendManager::self()->backend()->scanDevices(ScanFlag::includeLoopback);

    for (const auto &d : deviceList)
        operationStack().addDevice(d);

    operationStack().sortDevices();
}


#include "moc_devicescanner.cpp"
