/*
    SPDX-FileCopyrightText: 2023 Er2 <er2@dismail.de>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

/** @file
*/

#include "plugins/gpart/gpartbackend.h"
#include "plugins/gpart/gpartdevice.h"

#include "core/diskdevice.h"
#include "core/partition.h"
#include "core/partitiontable.h"

#include "fs/filesystemfactory.h"

#include "util/globallog.h"
#include "util/externalcommand.h"

#include <QString>
#include <QStringList>

#include <QFileInfo>

#include <KLocalizedString>
#include <KPluginFactory>

K_PLUGIN_CLASS_WITH_JSON(GpartBackend, "pmgpartbackendplugin.json")


GpartBackend::GpartBackend(QObject*, const QList<QVariant>&) :
    CoreBackend()
{
}

void GpartBackend::initFSSupport()
{
}

QList<Device*> GpartBackend::scanDevices(bool excludeReadOnly)
{
    return scanDevices(excludeReadOnly ? ScanFlags() : ScanFlag::includeReadOnly);
}

QList<Device*> GpartBackend::scanDevices(const ScanFlags scanFlags)
{
    //const bool includeReadOnly = scanFlags.testFlag(ScanFlag::includeReadOnly);
    const bool includeLoopback = scanFlags.testFlag(ScanFlag::includeLoopback);

    QList<Device*> result;
    QStringList deviceNodes;

    int error = geom_gettree(&m_mesh);
    if (error)
        return result;

    // Check https://foss.heptapod.net/bsdutils/bsdisks/-/blob/branch/default/geomprober.cpp
    gclass *c;
    ggeom *g;
    LIST_FOREACH(c, &m_mesh.lg_class, lg_class) {
        if (!strcmp(c->lg_name, "DISK")) {
            m_diskClass = c;
            LIST_FOREACH(g, &c->lg_geom, lg_geom) {
                deviceNodes << QString::fromLocal8Bit(g->lg_name);
            }
        }
        else if (includeLoopback && !strcmp(c->lg_name, "MD")) {
            m_mdClass = c;
            LIST_FOREACH(g, &c->lg_geom, lg_geom) {
                deviceNodes << QString::fromLocal8Bit(g->lg_name);
            }
        }
        else if (!strcmp(c->lg_name, "PART"))
            m_partClass = c;
    }

    int totalDevices = deviceNodes.length();
    for (int i = 0; i < totalDevices; ++i) {
        const QString deviceNode = deviceNodes[i];

        emitScanProgress(deviceNode, i * 100 / totalDevices);
        Device* device = scanDevice(deviceNode);
        if (device != nullptr) {
            result.append(device);
        }
    }

    geom_deletetree(&m_mesh);
    return result;
}

Device* GpartBackend::scanDevice(const QString &deviceNode)
{
    if (!m_partClass)
        return nullptr;

    ggeom *g;
    gprovider *p;
    DiskDevice *d = nullptr;

    const std::string nodeStr = deviceNode.toStdString();

    bool isMemoryDisk = deviceNode.startsWith(QStringLiteral("md"));
    gclass *c = m_diskClass;
    if (isMemoryDisk)
        c = m_mdClass;

    LIST_FOREACH(g, &c->lg_geom, lg_geom) {
        LIST_FOREACH(p, &g->lg_provider, lg_provider) {
            if (!strcmp(nodeStr.c_str(), p->lg_name)) {
                qint64 firstSector, lastSector;
                qint64 deviceSize = p->lg_mediasize;
                int logicalSectorSize = p->lg_sectorsize;

                QString name;

                gconfig *gc;
                LIST_FOREACH(gc, &p->lg_config, lg_config) {
                    QString val = QString::fromLocal8Bit(gc->lg_val);
                    if (!strcmp(gc->lg_name, "start")) {
                        firstSector = val.toULongLong();
                    }
                    else if (!strcmp(gc->lg_name, "last")) {
                        lastSector = val.toULongLong();
                    }
                    else if (!strcmp(gc->lg_name, "descr")) {
                        name = val;
                    }
                }

                if (name.isEmpty())
                    name = QString::fromLocal8Bit(p->lg_name);

                QString icon;
                // TODO: Icon selection
                if (isMemoryDisk)
                    icon = QStringLiteral("memory");
                else icon = QStringLiteral("drive-harddisk");
                d = new DiskDevice(name, QStringLiteral("/dev/") + deviceNode, 255, 63, deviceSize / logicalSectorSize / 255 / 63, logicalSectorSize, icon);


                setPartitionTableForDevice(*d, new PartitionTable(PartitionTable::TableType::none, firstSector, lastSector));
            }
        }
    }

    c = m_partClass;
    LIST_FOREACH(g, &c->lg_geom, lg_geom) {
        if (!strcmp(nodeStr.c_str(), g->lg_name)) {
            gconfig *gc;
            LIST_FOREACH(gc, &g->lg_config, lg_config) {
                QString val = QString::fromLocal8Bit(gc->lg_val);
                if (!strcmp(gc->lg_name, "scheme")) {
                    PartitionTable::TableType type = PartitionTable::TableType::none;
                    if (val == QStringLiteral("MBR")) type = PartitionTable::msdos;
                    else if (val == QStringLiteral("GPT")) type = PartitionTable::gpt;
                    if (d)
                        d->partitionTable()->setType(*d, type);
                }
                else if (!strcmp(gc->lg_name, "entries")) {
                    CoreBackend::setPartitionTableMaxPrimaries(*d->partitionTable(), val.toULongLong());
                }
            }
            LIST_FOREACH(p, &g->lg_provider, lg_provider) {
                if (d) {
                    QString partitionNode = QStringLiteral("/dev/") + QString::fromLocal8Bit(p->lg_name);
                    scanPartition(*d, partitionNode, p);
                }
            }
        }
    }

    if (d)
        d->partitionTable()->updateUnallocated(*d);

    return d;
}

void GpartBackend::scanPartition(Device& d, const QString& partitionNode, gprovider *p)
{
    qint64 firstSector, lastSector;
    QString partitionType, partitionUUID, partitionLabel;
    QString mountPoint;
    bool mounted = false;

    gconfig *gc;
    LIST_FOREACH(gc, &p->lg_config, lg_config) {
        QString val = QString::fromLocal8Bit(gc->lg_val);
        if (!strcmp(gc->lg_name, "start")) {
            firstSector = val.toULongLong();
        }
        else if (!strcmp(gc->lg_name, "end")) {
            lastSector = val.toULongLong();
        }
        else if (!strcmp(gc->lg_name, "type")) {
            partitionType = val;
        }
        else if (!strcmp(gc->lg_name, "rawuuid")) {
            partitionUUID = val;
        }
        else if (!strcmp(gc->lg_name, "label")) {
            partitionLabel = val;
        }
    }

    FileSystem::Type type = detectFileSystem(partitionNode);
    // FIXME: This fallbacks to swap detection
    if (partitionType == QStringLiteral("freebsd-swap")) type = FileSystem::Type::Unknown;

    PartitionTable::Flags activeFlags = PartitionTable::Flag::None;

    PartitionRole::Roles r = PartitionRole::Primary;

    if (type == FileSystem::Type::Unknown) {
        if (partitionType == QStringLiteral("ebr")) {
            r = PartitionRole::Extended;
            type = FileSystem::Type::Extended;
            //Device *d = scanDevice(partitionNode);
            //parent = d->partitionTable();
        }
        // FIXME: This isn't right but it works!
        else if (partitionType == QStringLiteral("freebsd-swap")) {
            type = FileSystem::Type::FreeBSDSwap;
            QFileInfo kernelPath(partitionNode);
            ExternalCommand cmd(QStringLiteral("swapctl"), {QStringLiteral("-l")});
            if (cmd.run(-1) && cmd.exitCode() == 0) {
                QByteArray data = cmd.rawOutput();

                QTextStream in(&data);
                while (!in.atEnd()) {
                    QStringList line = in.readLine().split(QRegularExpression(QStringLiteral("\\s+")));
                    if (line[0] == kernelPath.canonicalFilePath()) {
                        mounted = true;
                        break;
                    }
                }
            }
        }
        else if (partitionType == QStringLiteral("linux-swap")) {
            type = FileSystem::Type::LinuxSwap;
        }
        else if (partitionType == QStringLiteral("freebsd-zfs")) {
            type = FileSystem::Type::Zfs;
        }
        else if (partitionType == QStringLiteral("apple-apfs")) {
            type = FileSystem::Type::Apfs;
        }
    }

    PartitionNode* parent = d.partitionTable()->findPartitionBySector(firstSector, PartitionRole(PartitionRole::Extended));

    if (parent == nullptr)
        parent = d.partitionTable();
    else
        r = PartitionRole::Logical;

    FileSystem* fs = FileSystemFactory::create(type, firstSector, lastSector, d.logicalSize());
    fs->scan(partitionNode);

    mountPoint = FileSystem::detectMountPoint(fs, partitionNode);
    if (!mounted)
        mounted = FileSystem::detectMountStatus(fs, partitionNode);

    Partition* partition = new Partition(parent, d, PartitionRole(r), fs, firstSector, lastSector, partitionNode, availableFlags(d.partitionTable()->type()), mountPoint, mounted, activeFlags);

    if (!partitionLabel.isEmpty())
        partition->setLabel(partitionLabel);

    if (!partitionType.isEmpty())
        partition->setType(partitionType);

    if (!partitionUUID.isEmpty())
        partition->setUUID(partitionUUID);

    if (fs->supportGetLabel() != FileSystem::cmdSupportNone)
        fs->setLabel(fs->readLabel(partition->deviceNode()));

    if (fs->supportGetUUID() != FileSystem::cmdSupportNone)
        fs->setUUID(fs->readUUID(partition->deviceNode()));

    parent->append(partition);
}

FileSystem::Type GpartBackend::fileSystemNameToType(const QString &name)
{
    FileSystem::Type type = FileSystem::Type::Unknown;

    if (name == QStringLiteral("apfs\n")) type = FileSystem::Type::Apfs;
    //else if (name == QStringLiteral("befs\n")) type = FileSystem::Type::BeFs;
    else if (name == QStringLiteral("cd9660\n")) type = FileSystem::Type::Iso9660;
    else if (name == QStringLiteral("exfat\n")) type = FileSystem::Type::Exfat;
    else if (name == QStringLiteral("ext2fs\n")) type = FileSystem::Type::Ext2;
    // geli, hammer, hammer2?
    else if (name == QStringLiteral("hfs+\n")) type = FileSystem::Type::HfsPlus;
    else if (name == QStringLiteral("msdosfs\n")) type = FileSystem::Type::Fat32;
    else if (name == QStringLiteral("ntfs\n")) type = FileSystem::Type::Ntfs;
    else if (name == QStringLiteral("ufs\n")) type = FileSystem::Type::Ufs;
    else if (name == QStringLiteral("zfs\n")) type = FileSystem::Type::Zfs;

    return type;
}

FileSystem::Type GpartBackend::detectFileSystem(const QString& deviceNode)
{
    FileSystem::Type type = FileSystem::Type::Unknown;

    ExternalCommand fstypCommand(QStringLiteral("fstyp"), {deviceNode});
    if (fstypCommand.run(-1) && fstypCommand.exitCode() == 0) {
        QString fsType = fstypCommand.output();
        type = fileSystemNameToType(fsType);
    }

    //if (type == FileSystem::Type::Unknown) {
    //    qWarning() << "unknown file system type on " << deviceNode;
    //}
    return type;
}

QString GpartBackend::readLabel(const QString& deviceNode) const
{
    const std::string nodeStr = deviceNode.split(QStringLiteral("/dev/"))[1].toStdString();

    ggeom *g;
    gprovider *p;
    LIST_FOREACH(g, &m_partClass->lg_geom, lg_geom) {
        LIST_FOREACH(p, &g->lg_provider, lg_provider) {
            if (!strcmp(nodeStr.c_str(), p->lg_name)) {
                gconfig *gc;
                LIST_FOREACH(gc, &p->lg_config, lg_config) {
                    if (!strcmp(gc->lg_name, "label")) {
                        return QString::fromLocal8Bit(gc->lg_val);
                    }
                }
            }
        }
    }

    return QString();
}

QString GpartBackend::readUUID(const QString& deviceNode) const
{
    const std::string nodeStr = deviceNode.split(QStringLiteral("/dev/"))[1].toStdString();

    ggeom *g;
    gprovider *p;
    LIST_FOREACH(g, &m_partClass->lg_geom, lg_geom) {
        LIST_FOREACH(p, &g->lg_provider, lg_provider) {
            if (!strcmp(nodeStr.c_str(), p->lg_name)) {
                gconfig *gc;
                LIST_FOREACH(gc, &p->lg_config, lg_config) {
                    if (!strcmp(gc->lg_name, "rawuuid")) {
                        return QString::fromLocal8Bit(gc->lg_val);
                    }
                }
            }
        }
    }

    return QString();
}

PartitionTable::Flags GpartBackend::availableFlags(PartitionTable::TableType type)
{
    PartitionTable::Flags flags;
    if (type == PartitionTable::gpt) {
        // These are not really flags but for now keep them for compatibility
        // We should implement changing partition type
        flags = PartitionTable::Flag::BiosGrub |
                PartitionTable::Flag::Boot;
    }
    else if (type == PartitionTable::msdos || type == PartitionTable::msdos_sectorbased)
        flags = PartitionTable::Flag::Boot;

    return flags;
}

std::unique_ptr<CoreBackendDevice> GpartBackend::openDevice(const Device& d)
{
    std::unique_ptr<GpartDevice> device = std::make_unique<GpartDevice>(d);

    if (!device->open())
        device = nullptr;

    return device;
}

std::unique_ptr<CoreBackendDevice> GpartBackend::openDeviceExclusive(const Device& d)
{
    std::unique_ptr<GpartDevice> device = std::make_unique<GpartDevice>(d);

    if (!device->openExclusive())
        device = nullptr;

    return device;
}

bool GpartBackend::closeDevice(std::unique_ptr<CoreBackendDevice> coreDevice)
{
    return coreDevice->close();
}

#include "gpartbackend.moc"
