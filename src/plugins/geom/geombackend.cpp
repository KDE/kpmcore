/*
    SPDX-FileCopyrightText: 2023 Er2 <er2@dismail.de>
    SPDX-FileCopyrightText: 2025 Future Crew, LLC <license@futurecrew.ru>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

/** @file
*/

#include "plugins/geom/geombackend.h"
#include "plugins/geom/geomdevice.h"

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

K_PLUGIN_CLASS_WITH_JSON(GeomBackend, "pmgeombackendplugin.json")

static void forEachGeom(gmesh* mesh, const char* klass, const std::function<bool(ggeom*)>& f)
{
    gclass* geomClass = nullptr;
    ggeom* geom = nullptr;
    LIST_FOREACH(geomClass, &mesh->lg_class, lg_class) {
        if (::strcmp(geomClass->lg_name, klass))
            continue;

        bool cont;
        LIST_FOREACH(geom, &geomClass->lg_geom, lg_geom) {
            // No providers -> geom is being destroyed
            if (LIST_EMPTY(&geom->lg_provider))
                continue;

            cont = f(geom);
            if (!cont)
                break;
        }
    }
}

QList<DiskDevice*> GeomBackend::geomScan(bool includeLoopback, const QString& deviceNode)
{
    QHash<QString, DiskDevice*> devs;

    gmesh geomMesh;
    if (::geom_gettree(&geomMesh))
        return {};

    auto deviceFileName = deviceNode;
    deviceFileName.remove(QLatin1String("/dev/"));

    gclass* geomClass = nullptr;
    ggeom* geom = nullptr;
    LIST_FOREACH(geomClass, &geomMesh.lg_class, lg_class) {
        bool isMemory = ::strcmp(geomClass->lg_name, "MD") == 0;
        if (::strcmp(geomClass->lg_name, "DISK") != 0 && !(includeLoopback && isMemory))
            continue;
        LIST_FOREACH(geom, &geomClass->lg_geom, lg_geom) {
            QString geomName = QString::fromLocal8Bit(geom->lg_name);
            if(!deviceFileName.isEmpty() && deviceFileName != geomName)
                continue;

            // No providers -> geom is being destroyed
            if (LIST_EMPTY(&geom->lg_provider))
                continue;

            gprovider* geomProvider = geom->lg_provider.lh_first;
            qint64 sectors = 0;
            qint64 sectorSize = geomProvider->lg_sectorsize;
            QString diskName;

            gconfig* geomConfig = nullptr;
            bool withering = false;
            LIST_FOREACH(geomConfig, &geomProvider->lg_config, lg_config) {
                if (::strcmp(geomConfig->lg_name, "wither") == 0) {
                    withering = true;
                    break;
                }
                else if (::strcmp(geomConfig->lg_name, "fwsectors") == 0)
                    sectors = QString::fromLatin1(geomConfig->lg_val).toUInt();
                else if (::strcmp(geomConfig->lg_name, "descr") == 0)
                    diskName = QString::fromLocal8Bit(geomConfig->lg_val);
            }

            if (withering)
                continue;

            if (diskName.isEmpty())
                diskName = geomName;

            // TODO: ask CAM if we're using USB transport to display a fitting icon
            QString icon;
            if (isMemory)
                icon = QStringLiteral("memory");
            devs[geomName] = new DiskDevice(diskName, deviceNode, sectorSize, sectors, icon);

            if(!deviceFileName.isEmpty())
                goto loopEnd;
        }
    }
loopEnd:

    forEachGeom(&geomMesh, "PART", [&deviceFileName, &devs, this](auto* geom) {
        QString geomName = QString::fromLocal8Bit(geom->lg_name);
        if(!deviceFileName.isEmpty() && deviceFileName != geomName)
            return true;

        auto* d = devs.value(geomName, nullptr);
        if (!d)
            return true;

        qint64 firstSector, lastSector;
        QString scheme;
        gconfig* geomConfig = nullptr;
        bool withering = false;
        LIST_FOREACH(geomConfig, &geom->lg_config, lg_config) {
            if (::strcmp(geomConfig->lg_name, "wither") == 0) {
                withering = true;
                break;
            }
            else if (::strcmp(geomConfig->lg_name, "scheme") == 0)
                scheme = QString::fromLatin1(geomConfig->lg_val);
            else if (::strcmp(geomConfig->lg_name, "start") == 0)
                firstSector = QString::fromLatin1(geomConfig->lg_val).toULongLong();
            else if (::strcmp(geomConfig->lg_name, "last") == 0)
                lastSector = QString::fromLatin1(geomConfig->lg_val).toULongLong();
        }

        if (withering)
            return true;

        PartitionTable::TableType tableType = PartitionTable::TableType::unknownTableType;
        if (scheme == QStringLiteral("MBR"))
            tableType = PartitionTable::TableType::msdos;
        else if (scheme == QStringLiteral("GPT"))
            tableType = PartitionTable::TableType::gpt;

        setPartitionTableForDevice(*d, new PartitionTable(tableType, firstSector, lastSector));

        if (tableType == PartitionTable::TableType::unknownTableType)
            return deviceFileName.isEmpty(); // continue if we're scanning all devices

        gprovider *geomProvider;
        LIST_FOREACH(geomProvider, &geom->lg_provider, lg_provider) {
            QString partitionType, partitionUUID, partitionLabel;
            bool withering = false;
            LIST_FOREACH(geomConfig, &geomProvider->lg_config, lg_config) {
                if (::strcmp(geomConfig->lg_name, "wither") == 0) {
                    withering = true;
                    break;
                }
                else if (::strcmp(geomConfig->lg_name, "start") == 0) {
                    firstSector = QString::fromLatin1(geomConfig->lg_val).toULongLong();
                }
                else if (::strcmp(geomConfig->lg_name, "end") == 0) {
                    lastSector = QString::fromLatin1(geomConfig->lg_val).toULongLong();
                }
                else if (::strcmp(geomConfig->lg_name, "type") == 0) {
                    partitionType = QString::fromLatin1(geomConfig->lg_val);
                }
                else if (::strcmp(geomConfig->lg_name, "rawuuid") == 0) {
                    partitionUUID = QString::fromLatin1(geomConfig->lg_val);
                }
                else if (::strcmp(geomConfig->lg_name, "label") == 0) {
                    partitionLabel = QString::fromLocal8Bit(geomConfig->lg_val);
                }
            }

            if (withering)
                continue;

            QString partitionNode = QStringLiteral("/dev/") + QString::fromLatin1(geomProvider->lg_name);

            FileSystem::Type type = detectFileSystem(partitionNode);
            // FIXME: This fallbacks to swap detection
            if (partitionType == QStringLiteral("freebsd-swap")) type = FileSystem::Type::Unknown;

            PartitionTable::Flags activeFlags = PartitionTable::Flag::None;
            PartitionRole::Roles r = PartitionRole::Primary;
            bool mounted = false;

            if (type == FileSystem::Type::Unknown) {
                if (partitionType == QStringLiteral("ebr")) {
                    r = PartitionRole::Extended;
                    type = FileSystem::Type::Extended;
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

            PartitionNode* parent = d->partitionTable()->findPartitionBySector(firstSector, PartitionRole(PartitionRole::Extended));

            if (parent == nullptr)
                parent = d->partitionTable();
            else
                r = PartitionRole::Logical;

            FileSystem* fs = FileSystemFactory::create(type, firstSector, lastSector, d->logicalSize());
            fs->scan(partitionNode);

            QString mountPoint = FileSystem::detectMountPoint(fs, partitionNode);
            if (!mounted)
                mounted = FileSystem::detectMountStatus(fs, partitionNode);

            Partition* partition = new Partition(parent,
                                                 *d,
                                                 PartitionRole(r),
                                                 fs,
                                                 firstSector,
                                                 lastSector,
                                                 partitionNode,
                                                 availableFlags(d->partitionTable()->type()),
                                                 mountPoint,
                                                 mounted,
                                                 activeFlags);

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

        return deviceFileName.isEmpty(); // continue if we're scanning all devices
    });

    for (auto* d : std::as_const(devs))
        if (!d->partitionTable()) {
            const qint64 lastSector = d->totalLogical() - 1;
            setPartitionTableForDevice(*d, new PartitionTable(PartitionTable::TableType::none, 0, lastSector));

            FileSystem::Type type = detectFileSystem(d->deviceNode());
            FileSystem* fs = FileSystemFactory::create(type, 0, lastSector, d->logicalSize());
            fs->scan(d->deviceNode());

            QString mountPoint = FileSystem::detectMountPoint(fs, d->deviceNode());
            bool mounted = FileSystem::detectMountStatus(fs, d->deviceNode());

            Partition* partition = new Partition(d->partitionTable(),
                                                 *d,
                                                 PartitionRole(PartitionRole::Primary),
                                                 fs,
                                                 0,
                                                 lastSector,
                                                 d->deviceNode(),
                                                 availableFlags(d->partitionTable()->type()),
                                                 mountPoint,
                                                 mounted,
                                                 PartitionTable::Flag::None);

            d->partitionTable()->append(partition);
        }

    ::geom_deletetree(&geomMesh);

    return devs.values();
}

GeomBackend::GeomBackend(QObject*, const QList<QVariant>&) :
    CoreBackend()
{
}

void GeomBackend::initFSSupport()
{
}

QList<Device*> GeomBackend::scanDevices(bool excludeReadOnly)
{
    return scanDevices(excludeReadOnly ? ScanFlags() : ScanFlag::includeReadOnly);
}

QList<Device*> GeomBackend::scanDevices(const ScanFlags scanFlags)
{
    //const bool includeReadOnly = scanFlags.testFlag(ScanFlag::includeReadOnly);
    const bool includeLoopback = scanFlags.testFlag(ScanFlag::includeLoopback);
    QList<Device*> result;

    auto devs = geomScan(includeLoopback);
    for (auto* d : devs)
        result << static_cast<Device*>(d);

    return result;
}

Device* GeomBackend::scanDevice(const QString &deviceNode)
{
    auto devs = geomScan(true, deviceNode);
    if (devs.isEmpty())
        return nullptr;

    return static_cast<Device*>(*devs.begin());
}

FileSystem::Type GeomBackend::fileSystemNameToType(const QString &name)
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

FileSystem::Type GeomBackend::detectFileSystem(const QString& deviceNode)
{
    FileSystem::Type type = FileSystem::Type::Unknown;

    ExternalCommand fstypCommand(QStringLiteral("fstyp"), {deviceNode});
    if (fstypCommand.run(-1) && fstypCommand.exitCode() == 0) {
        QString fsType = fstypCommand.output();
        type = fileSystemNameToType(fsType);
    }

    return type;
}

QString GeomBackend::readLabel(const QString& deviceNode) const
{
    gmesh geomMesh;
    if (::geom_gettree(&geomMesh))
        return {};

    auto deviceFileName = deviceNode;
    deviceFileName.remove(QLatin1String("/dev/"));
    QString ret;

    forEachGeom(&geomMesh, "LABEL", [&deviceFileName, &ret](auto* geom) {
        QString geomName = QString::fromLocal8Bit(geom->lg_name);
        if (deviceFileName != geomName)
            return true;

        gprovider *geomProvider;
        LIST_FOREACH(geomProvider, &geom->lg_provider, lg_provider) {
            ret = QString::fromLocal8Bit(geomProvider->lg_name);
            return false;
        }

        return true;
    });

    ::geom_deletetree(&geomMesh);

    return ret;
}

QString GeomBackend::readUUID(const QString& deviceNode) const
{
    gmesh geomMesh;
    if (::geom_gettree(&geomMesh))
        return {};

    auto deviceFileName = deviceNode;
    deviceFileName.remove(QLatin1String("/dev/"));
    QString ret;

    forEachGeom(&geomMesh, "PART", [&deviceFileName, &ret](auto* geom) {
        QString geomName = QString::fromLocal8Bit(geom->lg_name);
        if (deviceFileName != geomName)
            return true;

        gprovider *geomProvider;
        gconfig *geomConfig;
        LIST_FOREACH(geomProvider, &geom->lg_provider, lg_provider) {
            LIST_FOREACH(geomConfig, &geomProvider->lg_config, lg_config) {
                if (::strcmp(geomConfig->lg_name, "rawuuid") == 0) {
                    ret = QString::fromLocal8Bit(geomConfig->lg_val);
                    return false;
                }
            }
        }

        return true;
    });

    ::geom_deletetree(&geomMesh);

    return {};
}

PartitionTable::Flags GeomBackend::availableFlags(PartitionTable::TableType type)
{
    PartitionTable::Flags flags;
    if (type == PartitionTable::gpt) {
        // These are not really flags but for now keep them for compatibility
        // We should implement changing partition type
        flags = PartitionTable::Flag::BiosGrub |
                PartitionTable::Flag::Boot;
    }
    else if (type == PartitionTable::msdos)
        flags = PartitionTable::Flag::Boot;

    return flags;
}

std::unique_ptr<CoreBackendDevice> GeomBackend::openDevice(const Device& d)
{
    std::unique_ptr<GeomDevice> device = std::make_unique<GeomDevice>(d);

    if (!device->open())
        device = nullptr;

    return device;
}

std::unique_ptr<CoreBackendDevice> GeomBackend::openDeviceExclusive(const Device& d)
{
    std::unique_ptr<GeomDevice> device = std::make_unique<GeomDevice>(d);

    if (!device->openExclusive())
        device = nullptr;

    return device;
}

bool GeomBackend::closeDevice(std::unique_ptr<CoreBackendDevice> coreDevice)
{
    return coreDevice->close();
}

#include "geombackend.moc"
