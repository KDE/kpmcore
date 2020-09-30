/*
    SPDX-FileCopyrightText: 2008-2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2014-2019 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2015 Teo Mrnjavac <teo@kde.org>
    SPDX-FileCopyrightText: 2016 Chantara Tith <tith.chantara@gmail.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

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

QList<Device*> DummyBackend::scanDevices(bool excludeReadOnly)
{
    Q_UNUSED(excludeReadOnly)
    return scanDevices(ScanFlags());
}

QList<Device*> DummyBackend::scanDevices(const ScanFlags scanFlags)
{
    Q_UNUSED(scanFlags)
    QList<Device*> result;
    result.append(scanDevice(QStringLiteral("/dev/sda")));

    emitScanProgress(QStringLiteral("/dev/sda"), 100);

    return scanDevices(false);
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

    return FileSystem::Type::Unknown;
}

QString DummyBackend::readLabel(const QString& deviceNode) const
{
    Q_UNUSED(deviceNode)

    return QString();
}

QString DummyBackend::readUUID(const QString& deviceNode) const
{
    Q_UNUSED(deviceNode)

    return QString();
}

std::unique_ptr<CoreBackendDevice> DummyBackend::openDevice(const Device& d)
{
    std::unique_ptr<DummyDevice> device = std::make_unique<DummyDevice>(d.deviceNode());

    if (!device->open())
        device = nullptr;

    return device;
}

std::unique_ptr<CoreBackendDevice> DummyBackend::openDeviceExclusive(const Device& d)
{
    std::unique_ptr<DummyDevice> device = std::make_unique<DummyDevice>(d.deviceNode());

    if (!device->openExclusive())
        device = nullptr;

    return device;
}

bool DummyBackend::closeDevice(std::unique_ptr<CoreBackendDevice> coreDevice)
{
    return coreDevice->close();
}

#include "dummybackend.moc"
