/*
    SPDX-FileCopyrightText: 2019 Shubham Jangra <aryan100jangid@gmail.com>
    SPDX-FileCopyrightText: 2019 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2019 Yuri Chornoivan <yurchor@ukr.net>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "testdevice.h" 
#include "helpers.h"

#include "backend/corebackend.h"
#include "backend/corebackendmanager.h"

#include <QtAlgorithms>
#include <QCoreApplication>
#include <QDebug>

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    
    KPMCoreInitializer init;
    
    if (argc == 2)
        init = KPMCoreInitializer(argv[1]);
    
    return init.isValid() ? EXIT_SUCCESS : EXIT_FAILURE;

    CoreBackend *backend = CoreBackendManager::self()->backend();
    
    if (!backend) {
        qWarning() << "Failed to load backend plugin";
        return EXIT_FAILURE;
    }
    
    TestDevice device;
    
    device.testDeviceName();
    device.testDeviceNode();
    device.testDeviceSize();
    device.testDeviceTotalSectors();
    
    return app.exec();
}

TestDevice::TestDevice()
{
    operationStack = new OperationStack();
    deviceScanner = new DeviceScanner(nullptr, *operationStack);
    deviceScanner->scan();
    
    // Get list of available devices on the system
    devices = operationStack->previewDevices(); 
}

TestDevice::~TestDevice()
{    
    delete operationStack;
    delete deviceScanner;
    
    // Delete the list of devices
    qDeleteAll(devices.begin(), devices.end());
    devices.clear();
}

void TestDevice::testDeviceName()
{
    if (devices.isEmpty()) {
        exit(EXIT_FAILURE);
    } else {
        for (const auto &device : devices) {
            if (device->name().isEmpty())
                exit(EXIT_FAILURE);
        }
    }
}

void TestDevice::testDeviceNode()
{
    if (devices.isEmpty()) {
        exit(EXIT_FAILURE);
    } else {
        for (const auto &device : devices) {
            if (device->deviceNode() == QString())
                exit(EXIT_FAILURE);
        }
    }
}

void TestDevice::testDeviceSize()
{
    if (devices.isEmpty()) {
        exit(EXIT_FAILURE);
    } else {
        for (const auto &device : devices) {
            if (device->logicalSize() < 0)
                exit(EXIT_FAILURE);
        }
    }
}

void TestDevice::testDeviceTotalSectors()
{
    if (devices.isEmpty()) {
        exit(EXIT_FAILURE);
    } else {
        for (const auto &device : devices) {
            if (device->totalLogical() < 0)
                exit(EXIT_FAILURE);
        }
    }
}
