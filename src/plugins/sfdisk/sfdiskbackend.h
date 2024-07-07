/*
    SPDX-FileCopyrightText: 2017-2020 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2018 Caio Jordão Carvalho <caiojcarvalho@gmail.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef SFDISKBACKEND__H
#define SFDISKBACKEND__H

#include "backend/corebackend.h"
#include "core/partition.h"
#include "fs/filesystem.h"

#include <QList>
#include <QVariant>

class Device;
class ExternalCommand;
class Partition;
class KPluginFactory;
class QString;

/** Backend plugin for sfdisk

    @author Andrius Štikonas <andrius@stikonas.eu>
*/
class SfdiskBackend : public CoreBackend
{
    Q_DISABLE_COPY(SfdiskBackend)

public:
    SfdiskBackend(QObject* parent, const QList<QVariant>& args);

public:
    void initFSSupport() override;

    QList<Device*> scanDevices(const ScanFlags scanFlags = {}) override;
    std::unique_ptr<CoreBackendDevice> openDevice(const Device& d) override;
    std::unique_ptr<CoreBackendDevice> openDeviceExclusive(const Device& d) override;
    bool closeDevice(std::unique_ptr<CoreBackendDevice> coreDevice) override;
    Device* scanDevice(const QString& deviceNode) override;
    FileSystem::Type detectFileSystem(const QString& partitionPath) override;
    QString readLabel(const QString& deviceNode) const override;
    QString readUUID(const QString& deviceNode) const override;

private:
    static void readSectorsUsed(const Device& d, Partition& p, const QString& mountPoint);
    void scanDevicePartitions(Device& d, const QJsonArray& jsonPartitions);
    Partition* scanPartition(Device& d, const QString& partitionNode, const qint64 firstSector, const qint64 lastSector, const QString& partitionType, const bool bootable);
    void scanWholeDevicePartition(Device& d);
    static void setupPartitionInfo(const Device& d, Partition* partition, const QJsonObject& partitionObject);
    bool updateDevicePartitionTable(Device& d, const QJsonObject& jsonPartitionTable);
    static PartitionTable::Flags availableFlags(PartitionTable::TableType type);
    static FileSystem::Type fileSystemNameToType(const QString& fileSystemName, const QString& version);
    static FileSystem::Type runDetectFileSystemCommand(ExternalCommand& command, QString& typeRegExp, QString& versionRegExp, QString& name);
};

#endif
