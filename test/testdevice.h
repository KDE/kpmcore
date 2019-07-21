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
**************************************************************************/
 
//  SPDX-License-Identifier: GPL-3.0+ 
 
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

    bool testDeviceName();
    bool testDeviceNode();
    bool testDeviceSize();
    bool testDeviceTotalSectors();
    
private:
    OperationStack *operationStack;
    DeviceScanner *deviceScanner;
    QList <Device*> devices;
};

#endif // TESTDEVICE_H
