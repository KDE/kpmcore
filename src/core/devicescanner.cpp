/*************************************************************************
 *  Copyright (C) 2010 by Volker Lanz <vl@fidra.de>                      *
 *  Copyright (C) 2016 by Andrius Štikonas <andrius@stikonas.eu>         *
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

#include "core/devicescanner.h"

#include "backend/corebackend.h"
#include "backend/corebackendmanager.h"

#include "core/operationstack.h"
#include "core/device.h"
#include "core/lvmdevice.h"
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
    emit progress(QString(), 0);

    clear();

    const QList<Device*> deviceList = CoreBackendManager::self()->backend()->scanDevices();
    const QList<LvmDevice*> lvmList = LvmDevice::scanSystemLVM();

    // Some LVM operations require additional information about LVM physical volumes which we store in LVM::pvList
    LVM::pvList = FS::lvm2_pv::getPVs(deviceList);

    for (const auto &d : deviceList)
        operationStack().addDevice(d);

    // Display alphabetically sorted disk devices above LVM VGs
    operationStack().sortDevices();

    // Look for LVM physical volumes in LVM VGs
    for (const auto &d : lvmList) {
        operationStack().addDevice(d);
        LVM::pvList.append(FS::lvm2_pv::getPVinNode(d->partitionTable()));
    }

    // Inform LvmDevice about which physical volumes form that particular LvmDevice
    for (const auto &d : lvmList)
        for (const auto &p : qAsConst(LVM::pvList))
            if (p.vgName() == d->name())
                d->physicalVolumes().append(p.partition());
}

