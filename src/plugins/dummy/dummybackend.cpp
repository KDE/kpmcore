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

/** @file
*/

#include "plugins/dummy/dummybackend.h"
#include "plugins/dummy/dummydevice.h"

#include "core/diskdevice.h"
#include "core/partition.h"
#include "core/partitiontable.h"

#include "util/globallog.h"

#include <QString>
#include <QStringList>

#include <KLocalizedString>
#include <KPluginFactory>

K_PLUGIN_FACTORY_WITH_JSON(DummyBackendFactory, "pmdummybackendplugin.json", registerPlugin<DummyBackend>();)


DummyBackend::DummyBackend(QObject*, const QList<QVariant>&) :
    CoreBackend()
{
}

void DummyBackend::initFSSupport()
{
}

QList<Device*> DummyBackend::scanDevices(bool excludeLoop)
{
    Q_UNUSED(excludeLoop)
    QList<Device*> result;
    result.append(scanDevice(QStringLiteral("/dev/sda")));

    emitScanProgress(QStringLiteral("/dev/sda"), 100);

    return result;
}

Device* DummyBackend::scanDevice(const QString& deviceNode)
{
    DiskDevice* d = new DiskDevice(QStringLiteral("Dummy Device"), QStringLiteral("/tmp") + deviceNode, 255, 30, 63, 512);
    CoreBackend::setPartitionTableForDevice(*d, new PartitionTable(PartitionTable::msdos_sectorbased, 2048, d->totalSectors() - 2048));
    CoreBackend::setPartitionTableMaxPrimaries(*d->partitionTable(), 128);
    d->partitionTable()->updateUnallocated(*d);
    d->setIconName(QStringLiteral("drive-harddisk"));

    CoreBackend::setPartitionTableMaxPrimaries(*d->partitionTable(), 4);

    return d;
}

FileSystem::Type DummyBackend::detectFileSystem(const QString& deviceNode)
{
    Q_UNUSED(deviceNode)

    return FileSystem::Unknown;
}

CoreBackendDevice* DummyBackend::openDevice(const Device& d)
{
    DummyDevice* device = new DummyDevice(d.deviceNode());

    if (device == nullptr || !device->open()) {
        delete device;
        device = nullptr;
    }

    return device;
}

CoreBackendDevice* DummyBackend::openDeviceExclusive(const Device& d)
{
    DummyDevice* device = new DummyDevice(d.deviceNode());

    if (device == nullptr || !device->openExclusive()) {
        delete device;
        device = nullptr;
    }

    return device;
}

bool DummyBackend::closeDevice(CoreBackendDevice* coreDevice)
{
    return coreDevice->close();
}

#include "dummybackend.moc"
