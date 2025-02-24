/*
    SPDX-FileCopyrightText: 2023 Er2 <er2@dismail.de>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KPMCORE_GEOMBACKEND_H
#define KPMCORE_GEOMBACKEND_H

#include "backend/corebackend.h"
#include "core/partition.h"

#include <QList>
#include <QVariant>

#include <libgeom.h>

class Device;
class KPluginFactory;
class QString;

/** Geom backend plugin for FreeBSD.

    @author Er2 <er2@dismail.de>
*/
class GeomBackend : public CoreBackend
{
    Q_DISABLE_COPY(GeomBackend)

public:
    GeomBackend(QObject* parent, const QList<QVariant>& args);

public:
    void initFSSupport() override;

    QList<Device*> scanDevices(bool excludeReadOnly = false);
    QList<Device*> scanDevices(const ScanFlags scanFlags) override;
    std::unique_ptr<CoreBackendDevice> openDevice(const Device& d) override;
    std::unique_ptr<CoreBackendDevice> openDeviceExclusive(const Device& d) override;
    bool closeDevice(std::unique_ptr<CoreBackendDevice> coreDevice) override;
    Device* scanDevice(const QString& deviceNode) override;
    FileSystem::Type detectFileSystem(const QString& deviceNode) override;
    QString readLabel(const QString& deviceNode) const override;
    QString readUUID(const QString& deviceNode) const override;

private:
    gmesh m_mesh;
    gclass *m_diskClass;
    gclass *m_partClass;
    gclass *m_mdClass;

    static PartitionTable::Flags availableFlags(PartitionTable::TableType type);
    void scanPartition(Device& d, const QString& partitionNode, gprovider *p);
    FileSystem::Type fileSystemNameToType(const QString &name);
};

#endif
