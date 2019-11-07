 /*************************************************************************
 *  Copyright (C) 2019 by Shubham <aryan100jangid@gmail.com>             *
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
 
 //  SPDX-License-Identifier: GPL-3.0+

#include "helpers.h"
#include "testdevice.h" 

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
            if (device->name() == QString())
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
