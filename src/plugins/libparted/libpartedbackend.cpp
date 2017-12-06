/*************************************************************************
 *  Copyright (C) 2008-2012 by Volker Lanz <vl@fidra.de>                 *
 *  Copyright (C) 2015-2016 by Teo Mrnjavac <teo@kde.org>                *
 *  Copyright (C) 2016-2017 by Andrius Štikonas <andrius@stikonas.eu>    *
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

#include "plugins/libparted/libpartedbackend.h"
#include "plugins/libparted/libparteddevice.h"
#include "plugins/libparted/pedflags.h"

#include "core/lvmdevice.h"
#include "core/partition.h"
#include "core/partitiontable.h"
#include "core/partitionalignment.h"

#include "fs/filesystem.h"
#include "fs/filesystemfactory.h"

#include "fs/fat16.h"
#include "fs/hfs.h"
#include "fs/hfsplus.h"
#include "fs/luks.h"
#include "fs/lvm2_pv.h"

#include "util/globallog.h"
#include "util/externalcommand.h"
#include "util/helpers.h"

#include <blkid/blkid.h>

#include <QDebug>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStorageInfo>
#include <QString>
#include <QStringList>

#include <KLocalizedString>
#include <KPluginFactory>

#include <parted/parted.h>

K_PLUGIN_FACTORY_WITH_JSON(LibPartedBackendFactory, "pmlibpartedbackendplugin.json", registerPlugin<LibPartedBackend>();)

static QString s_lastPartedExceptionMessage;

/** Callback to handle exceptions from libparted
    @param e the libparted exception to handle
*/
static PedExceptionOption pedExceptionHandler(PedException* e)
{
    Log(Log::error) << xi18nc("@info:status", "LibParted Exception: %1", QString::fromLocal8Bit(e->message));
    s_lastPartedExceptionMessage = QString::fromLocal8Bit(e->message);
    return PED_EXCEPTION_UNHANDLED;
}

// --------------------------------------------------------------------------

// The following structs and the typedef come from libparted's internal gpt sources.
// It's very unfortunate there is no public API to get at the first and last usable
// sector for GPT a partition table, so this is the only (libparted) way to get that
// information (another way would be to read the GPT header and parse the
// information ourselves; if the libparted devs begin changing these internal
// structs for each point release and break our code, we'll have to do just that).

typedef struct {
    uint32_t time_low;
    uint16_t time_mid;
    uint16_t time_hi_and_version;
    uint8_t  clock_seq_hi_and_reserved;
    uint8_t  clock_seq_low;
    uint8_t  node[6];
} /* __attribute__ ((packed)) */ efi_guid_t;


struct __attribute__((packed)) _GPTDiskData {
    PedGeometry data_area;
    int     entry_count;
    efi_guid_t  uuid;
};

typedef struct _GPTDiskData GPTDiskData;

// --------------------------------------------------------------------------

/** Get the first sector a Partition may cover on a given Device
    @param d the Device in question
    @return the first sector usable by a Partition
*/
static qint64 firstUsableSector(const Device& d)
{
    PedDevice* pedDevice = ped_device_get(d.deviceNode().toLocal8Bit().constData());
    PedDisk* pedDisk = pedDevice ? ped_disk_new(pedDevice) : nullptr;

    qint64 rval = 0;
    if (pedDisk)
        rval = static_cast<qint64>(pedDisk->dev->bios_geom.sectors);

    if (pedDisk && strcmp(pedDisk->type->name, "gpt") == 0) {
        GPTDiskData* gpt_disk_data = reinterpret_cast<GPTDiskData*>(pedDisk->disk_specific);
        PedGeometry* geom = reinterpret_cast<PedGeometry*>(&gpt_disk_data->data_area);

        if (geom)
            rval = static_cast<qint64>(geom->start);
        else
            rval += 32;
    }

    ped_disk_destroy(pedDisk);

    return rval;
}

/** Get the last sector a Partition may cover on a given Device
    @param d the Device in question
    @return the last sector usable by a Partition
*/
static qint64 lastUsableSector(const Device& d)
{
    PedDevice* pedDevice = ped_device_get(d.deviceNode().toLocal8Bit().constData());
    PedDisk* pedDisk = pedDevice ? ped_disk_new(pedDevice) : nullptr;

    qint64 rval = 0;
    if (pedDisk)
        rval = static_cast< qint64 >( pedDisk->dev->bios_geom.sectors ) *
               static_cast< qint64 >( pedDisk->dev->bios_geom.heads ) *
               static_cast< qint64 >( pedDisk->dev->bios_geom.cylinders - 1 );

    if (pedDisk && strcmp(pedDisk->type->name, "gpt") == 0) {
        GPTDiskData* gpt_disk_data = reinterpret_cast<GPTDiskData*>(pedDisk->disk_specific);
        PedGeometry* geom = reinterpret_cast<PedGeometry*>(&gpt_disk_data->data_area);

        if (geom)
            rval = geom->end;
        else
            rval -= 32;
    }

    ped_disk_destroy(pedDisk);

    return rval;
}

/** Reads sectors used on a FileSystem using libparted functions.
    @param pedDisk pointer to pedDisk  where the Partition and its FileSystem are
    @param p the Partition the FileSystem is on
    @return the number of sectors used
*/
#if defined LIBPARTED_FS_RESIZE_LIBRARY_SUPPORT
static qint64 readSectorsUsedLibParted(PedDisk* pedDisk, const Partition& p)
{
    Q_ASSERT(pedDisk);

    qint64 rval = -1;

    PedPartition* pedPartition = ped_disk_get_partition_by_sector(pedDisk, p.firstSector());

    if (pedPartition) {
        PedFileSystem* pedFileSystem = ped_file_system_open(&pedPartition->geom);

        if (pedFileSystem) {
            if (PedConstraint* pedConstraint = ped_file_system_get_resize_constraint(pedFileSystem)) {
                rval = pedConstraint->min_size;
                ped_constraint_destroy(pedConstraint);
            }

            ped_file_system_close(pedFileSystem);
        }
    }

    return rval;
}
#endif

/** Reads the sectors used in a FileSystem and stores the result in the Partition's FileSystem object.
    @param pedDisk pointer to pedDisk  where the Partition and its FileSystem are
    @param p the Partition the FileSystem is on
    @param mountPoint mount point of the partition in question
*/
static void readSectorsUsed(PedDisk* pedDisk, const Device& d, Partition& p, const QString& mountPoint)
{
    if (!mountPoint.isEmpty() && p.fileSystem().type() != FileSystem::LinuxSwap && p.fileSystem().type() != FileSystem::Lvm2_PV) {
        const QStorageInfo storage = QStorageInfo(mountPoint);
        if (p.isMounted() && storage.isValid())
            p.fileSystem().setSectorsUsed( (storage.bytesTotal() - storage.bytesFree()) / d.logicalSize());
    }
    else if (p.fileSystem().supportGetUsed() == FileSystem::cmdSupportFileSystem)
        p.fileSystem().setSectorsUsed(p.fileSystem().readUsedCapacity(p.deviceNode()) / d.logicalSize());
#if defined LIBPARTED_FS_RESIZE_LIBRARY_SUPPORT
    else if (p.fileSystem().supportGetUsed() == FileSystem::cmdSupportCore)
        p.fileSystem().setSectorsUsed(readSectorsUsedLibParted(pedDisk, p));
#else
        Q_UNUSED(pedDisk);
#endif
}

static PartitionTable::Flags activeFlags(PedPartition* p)
{
    PartitionTable::Flags flags = PartitionTable::FlagNone;

    // We might get here with a pedPartition just picked up from libparted that is
    // unallocated. Libparted doesn't like it if we ask for flags for unallocated
    // space.
    if (p->num <= 0)
        return flags;

    for (const auto &flag : flagmap)
        if (ped_partition_is_flag_available(p, flag.pedFlag) && ped_partition_get_flag(p, flag.pedFlag))
            flags |= flag.flag;

    return flags;
}

static PartitionTable::Flags availableFlags(PedPartition* p)
{
    PartitionTable::Flags flags;

    // see above.
    if (p->num <= 0)
        return flags;

    for (const auto &flag : flagmap) {
        if (ped_partition_is_flag_available(p, flag.pedFlag)) {
            // Workaround: libparted claims the hidden flag is available for extended partitions, but
            // throws an error when we try to set or clear it. So skip this combination. Also see setFlag.
            if (p->type != PED_PARTITION_EXTENDED || flag.flag != PartitionTable::FlagHidden)
                flags |= flag.flag;
        }
    }

    return flags;
}

/** Constructs a LibParted object. */
LibPartedBackend::LibPartedBackend(QObject*, const QList<QVariant>&) :
    CoreBackend()
{
    ped_exception_set_handler(pedExceptionHandler);
}

void LibPartedBackend::initFSSupport()
{
#if defined LIBPARTED_FS_RESIZE_LIBRARY_SUPPORT
    if (FS::fat16::m_Shrink == FileSystem::cmdSupportNone)
        FS::fat16::m_Shrink = FileSystem::cmdSupportBackend;

    if (FS::fat16::m_Grow == FileSystem::cmdSupportNone)
        FS::fat16::m_Grow = FileSystem::cmdSupportBackend;

    if (FS::hfs::m_Shrink == FileSystem::cmdSupportNone)
        FS::hfs::m_Shrink = FileSystem::cmdSupportBackend;

    if (FS::hfsplus::m_Shrink == FileSystem::cmdSupportNone)
        FS::hfsplus::m_Shrink = FileSystem::cmdSupportBackend;

    if (FS::hfs::m_GetUsed == FileSystem::cmdSupportNone)
        FS::hfs::m_GetUsed = FileSystem::cmdSupportBackend;

    if (FS::hfsplus::m_GetUsed == FileSystem::cmdSupportNone)
        FS::hfsplus::m_GetUsed = FileSystem::cmdSupportBackend;
#endif
}

/** Scans a Device for Partitions.

    This method  will scan a Device for all Partitions on it, detect the FileSystem for each Partition,
    try to determine the FileSystem usage, read the FileSystem label and store it all in newly created
    objects that are in the end added to the Device's PartitionTable.

    @param d Device
    @param pedDisk libparted pointer to the partition table
*/
void LibPartedBackend::scanDevicePartitions(Device& d, PedDisk* pedDisk)
{
    Q_ASSERT(pedDisk);
    Q_ASSERT(d.partitionTable());

    PedPartition* pedPartition = nullptr;

    QList<Partition*> partitions;

    while ((pedPartition = ped_disk_next_partition(pedDisk, pedPartition))) {
        if (pedPartition->num < 1)
            continue;

        PartitionRole::Roles r = PartitionRole::None;

        FileSystem::Type type = FileSystem::Unknown;
        char* pedPath = ped_partition_get_path(pedPartition);
        const QString partitionNode = pedPath ? QString::fromLocal8Bit(pedPath) : QString();
        free(pedPath);
        type = detectFileSystem(partitionNode);

        switch (pedPartition->type) {
        case PED_PARTITION_NORMAL:
            r = PartitionRole::Primary;
            break;

        case PED_PARTITION_EXTENDED:
            r = PartitionRole::Extended;
            type = FileSystem::Extended;
            break;

        case PED_PARTITION_LOGICAL:
            r = PartitionRole::Logical;
            break;

        default:
            continue;
        }

        // Find an extended partition this partition is in.
        PartitionNode* parent = d.partitionTable()->findPartitionBySector(pedPartition->geom.start, PartitionRole(PartitionRole::Extended));

        // None found, so it's a primary in the device's partition table.
        if (parent == nullptr)
            parent = d.partitionTable();

        FileSystem* fs = FileSystemFactory::create(type, pedPartition->geom.start, pedPartition->geom.end, d.logicalSize());
        fs->scan(partitionNode);
        QString mountPoint;
        bool mounted;

        // libparted does not handle LUKS partitions
        if (fs->type() == FileSystem::Luks) {
            r |= PartitionRole::Luks;
            FS::luks* luksFs = static_cast<FS::luks*>(fs);
            luksFs->initLUKS();
            QString mapperNode = luksFs->mapperName();
            mountPoint = FileSystem::detectMountPoint(fs, mapperNode);
            mounted    = FileSystem::detectMountStatus(fs, mapperNode);
        } else {
            mountPoint = FileSystem::detectMountPoint(fs, partitionNode);
            mounted = FileSystem::detectMountStatus(fs, partitionNode);
        }

        Partition* part = new Partition(parent, d, PartitionRole(r), fs, pedPartition->geom.start, pedPartition->geom.end, partitionNode, availableFlags(pedPartition), mountPoint, mounted, activeFlags(pedPartition));

        if (!part->roles().has(PartitionRole::Luks))
            readSectorsUsed(pedDisk, d, *part, mountPoint);

        if (fs->supportGetLabel() != FileSystem::cmdSupportNone)
            fs->setLabel(fs->readLabel(part->deviceNode()));

        // GPT partitions support partition labels and partition UUIDs
        if(d.partitionTable()->type() == PartitionTable::TableType::gpt)
            part->setLabel(QLatin1String(ped_partition_get_name(pedPartition)));

        if (fs->supportGetUUID() != FileSystem::cmdSupportNone)
            fs->setUUID(fs->readUUID(part->deviceNode()));

        parent->append(part);
        partitions.append(part);
    }

    d.partitionTable()->updateUnallocated(d);

    if (d.partitionTable()->isSectorBased(d))
        d.partitionTable()->setType(d, PartitionTable::msdos_sectorbased);

    for (const Partition * part : qAsConst(partitions))
        PartitionAlignment::isAligned(d, *part);
}

/** Create a Device for the given device_node and scan it for partitions.
    @param deviceNode the device node (e.g. "/dev/sda")
    @return the created Device object. callers need to free this.
*/
DiskDevice* LibPartedBackend::scanDevice(const QString& deviceNode)
{
    PedDevice* pedDevice = ped_device_get(deviceNode.toLocal8Bit().constData());

    if (pedDevice == nullptr) {
        Log(Log::warning) << xi18nc("@info:status", "Could not access device <filename>%1</filename>", deviceNode);
        return nullptr;
    }

    Log(Log::information) << xi18nc("@info:status", "Device found: %1", QString::fromLocal8Bit(pedDevice->model));

    DiskDevice* d = new DiskDevice(QString::fromLocal8Bit(pedDevice->model), QString::fromLocal8Bit(pedDevice->path), pedDevice->bios_geom.heads, pedDevice->bios_geom.sectors, pedDevice->bios_geom.cylinders, pedDevice->sector_size);

    PedDisk* pedDisk = ped_disk_new(pedDevice);

    if (pedDisk) {
        const PartitionTable::TableType type = PartitionTable::nameToTableType(QString::fromLocal8Bit(pedDisk->type->name));
        CoreBackend::setPartitionTableForDevice(*d, new PartitionTable(type, firstUsableSector(*d), lastUsableSector(*d)));
        CoreBackend::setPartitionTableMaxPrimaries(*d->partitionTable(), ped_disk_get_max_primary_partition_count(pedDisk));

        scanDevicePartitions(*d, pedDisk);
    }

    ped_device_destroy(pedDevice);
    return d;
}

QList<Device*> LibPartedBackend::scanDevices(bool excludeReadOnly)
{
//  TODO: add another bool option for loopDevices
    QList<Device*> result;
    QStringList deviceNodes;

    ExternalCommand cmd(QStringLiteral("lsblk"),
                        { QStringLiteral("--nodeps"),
                          QStringLiteral("--paths"),
                          QStringLiteral("--sort"), QStringLiteral("name"),
                          QStringLiteral("--json"),
                          QStringLiteral("--output"),
                          QStringLiteral("type,name") });

    if (cmd.run(-1) && cmd.exitCode() == 0) {
        const QJsonDocument jsonDocument = QJsonDocument::fromJson(cmd.rawOutput());
        QJsonObject jsonObject = jsonDocument.object();
        const QJsonArray jsonArray = jsonObject[QLatin1String("blockdevices")].toArray();
        for (const auto &deviceLine : jsonArray) {
            QJsonObject deviceObject = deviceLine.toObject();
            if (deviceObject[QLatin1String("type")].toString() != QLatin1String("disk"))
                continue;

            const QString deviceNode = deviceObject[QLatin1String("name")].toString();
            if (excludeReadOnly) {
                QString deviceName = deviceNode;
                deviceName.remove(QStringLiteral("/dev/"));
                QFile f(QStringLiteral("/sys/block/%1/ro").arg(deviceName));
                if (f.open(QIODevice::ReadOnly))
                    if (f.readLine().trimmed().toInt() == 1)
                        continue;
            }
            deviceNodes << deviceNode;
        }

        int totalDevices = deviceNodes.length();
        for (int i = 0; i < totalDevices; ++i) {
            const QString deviceNode = deviceNodes[i];

            emitScanProgress(deviceNode, i * 100 / totalDevices);
            Device* device = scanDevice(deviceNode);
            if(device != nullptr) {
                result.append(device);
            }
        }

        LvmDevice::scanSystemLVM(result);
    }

    return result;
}

/** Detects the type of a FileSystem given a PedDevice and a PedPartition
    @param partitionPath path to the partition
    @return the detected FileSystem type (FileSystem::Unknown if not detected)
*/
FileSystem::Type LibPartedBackend::detectFileSystem(const QString& partitionPath)
{
    FileSystem::Type rval = FileSystem::Unknown;

    blkid_cache cache;
    if (blkid_get_cache(&cache, nullptr) == 0) {
        blkid_dev dev;

        if ((dev = blkid_get_dev(cache,
                                 partitionPath.toLocal8Bit().constData(),
                                 BLKID_DEV_NORMAL)) != nullptr) {
            char *string = blkid_get_tag_value(cache, "TYPE", partitionPath.toLocal8Bit().constData());
            QString s = QString::fromLocal8Bit(string);
            free(string);

            if (s == QStringLiteral("ext2")) rval = FileSystem::Ext2;
            else if (s == QStringLiteral("ext3")) rval = FileSystem::Ext3;
            else if (s.startsWith(QStringLiteral("ext4"))) rval = FileSystem::Ext4;
            else if (s == QStringLiteral("swap")) rval = FileSystem::LinuxSwap;
            else if (s == QStringLiteral("ntfs")) rval = FileSystem::Ntfs;
            else if (s == QStringLiteral("reiserfs")) rval = FileSystem::ReiserFS;
            else if (s == QStringLiteral("reiser4")) rval = FileSystem::Reiser4;
            else if (s == QStringLiteral("xfs")) rval = FileSystem::Xfs;
            else if (s == QStringLiteral("jfs")) rval = FileSystem::Jfs;
            else if (s == QStringLiteral("hfs")) rval = FileSystem::Hfs;
            else if (s == QStringLiteral("hfsplus")) rval = FileSystem::HfsPlus;
            else if (s == QStringLiteral("ufs")) rval = FileSystem::Ufs;
            else if (s == QStringLiteral("vfat")) {
                // libblkid uses SEC_TYPE to distinguish between FAT16 and FAT32
                string = blkid_get_tag_value(cache, "SEC_TYPE", partitionPath.toLocal8Bit().constData());
                QString st = QString::fromLocal8Bit(string);
                free(string);
                if (st == QStringLiteral("msdos"))
                    rval = FileSystem::Fat16;
                else
                    rval = FileSystem::Fat32;
            } else if (s == QStringLiteral("btrfs")) rval = FileSystem::Btrfs;
            else if (s == QStringLiteral("ocfs2")) rval = FileSystem::Ocfs2;
            else if (s == QStringLiteral("zfs_member")) rval = FileSystem::Zfs;
            else if (s == QStringLiteral("hpfs")) rval = FileSystem::Hpfs;
            else if (s == QStringLiteral("crypto_LUKS")) rval = FileSystem::Luks;
            else if (s == QStringLiteral("exfat")) rval = FileSystem::Exfat;
            else if (s == QStringLiteral("nilfs2")) rval = FileSystem::Nilfs2;
            else if (s == QStringLiteral("LVM2_member")) rval = FileSystem::Lvm2_PV;
            else if (s == QStringLiteral("f2fs")) rval = FileSystem::F2fs;
            else if (s == QStringLiteral("udf")) rval = FileSystem::Udf;
            else if (s == QStringLiteral("iso9660")) rval = FileSystem::Iso9660;
            else
                qWarning() << "blkid: unknown file system type " << s << " on " << partitionPath;
        }

        blkid_put_cache(cache);
    }

    return rval;
}

CoreBackendDevice* LibPartedBackend::openDevice(const Device& d)
{
    LibPartedDevice* device = new LibPartedDevice(d.deviceNode());

    if (device == nullptr || !device->open()) {
        delete device;
        device = nullptr;
    }

    return device;
}

CoreBackendDevice* LibPartedBackend::openDeviceExclusive(const Device& d)
{
    LibPartedDevice* device = new LibPartedDevice(d.deviceNode());

    if (device == nullptr || !device->openExclusive()) {
        delete device;
        device = nullptr;
    }

    return device;
}

bool LibPartedBackend::closeDevice(CoreBackendDevice* core_device)
{
    return core_device->close();
}

PedPartitionFlag LibPartedBackend::getPedFlag(PartitionTable::Flag flag)
{
    for (const auto &f : flagmap)
        if (f.flag == flag)
            return f.pedFlag;

    return static_cast<PedPartitionFlag>(-1);
}

QString LibPartedBackend::lastPartedExceptionMessage()
{
    return s_lastPartedExceptionMessage;
}

#include "libpartedbackend.moc"
