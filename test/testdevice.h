/*
    SPDX-FileCopyrightText: 2019 Shubham Jangra <aryan100jangid@gmail.com>
    SPDX-FileCopyrightText: 2019 Andrius Štikonas <andrius@stikonas.eu>

    SPDX-License-Identifier: GPL-3.0-or-later
*/
 
#ifndef TESTDEVICE_H
#define TESTDEVICE_H

#include "core/device.h"
#include "core/devicescanner.h"
#include "core/operationstack.h" 
 
#include <QList> 
 
class TestDevice
{
public:
    TestDevice();
   ~TestDevice();

    void testDeviceName();
    void testDeviceNode();
    void testDeviceSize();
    void testDeviceTotalSectors();
    
private:
    OperationStack *operationStack;
    DeviceScanner *deviceScanner;
    QList <Device*> devices;
};

#endif // TESTDEVICE_H
