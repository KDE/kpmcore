/*************************************************************************
 *  Copyright (C) 2008-2012 by Volker Lanz <vl@fidra.de>                 *
 *  Copyright (C) 2015-2016 by Teo Mrnjavac <teo@kde.org>                *
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

#include "plugins/libparted/libpartedbackend.h"
#include "plugins/libparted/libparteddevice.h"
#include "plugins/libparted/pedflags.h"

#include "core/device.h"
#include "core/partition.h"
#include "core/partitiontable.h"
#include "core/partitionalignment.h"

#include "fs/filesystem.h"
#include "fs/filesystemfactory.h"

#include "fs/fat16.h"
#include "fs/hfs.h"
#include "fs/hfsplus.h"
#include "fs/luks.h"

#include "util/globallog.h"
#include "util/externalcommand.h"
#include "util/helpers.h"

#include <blkid/blkid.h>

#include <QDebug>
#include <QRegularExpressionMatch>
#include <QString>
#include <QStringList>

#include <KAuth>
#include <KLocalizedString>
#include <KMountPoint>
#include <KDiskFreeSpaceInfo>
#include <KPluginFactory>

#include <unistd.h>

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

/** Reads sectors used on a FileSystem using libparted functions.
    @param pedDisk pointer to pedDisk  where the Partition and its FileSystem are
    @param p the Partition the FileSystem is on
    @return the number of sectors used
*/
#if defined LIBPARTED_FS_RESIZE_LIBRARY_SUPPORT
static qint64 readSectorsUsedLibParted(const Partition& p)
{
    QVariantMap args;
    args[QLatin1String("deviceNode")] = p.deviceNode();
    args[QLatin1String("firstSector")] = p.firstSector();

    KAuth::Action scanAction = QStringLiteral("org.kde.kpmcore.scan.readsectorsused");
    scanAction.setHelperId(QStringLiteral("org.kde.kpmcore.scan"));
    scanAction.setArguments(args);
    KAuth::ExecuteJob *job = scanAction.execute();
    if (!job->exec()) {
        qWarning() << "KAuth returned an error code: " << job->errorString();
        return -1;
    }

    return job->data()[QLatin1String("sectorsUsed")].toLongLong();
}
#endif

/** Reads the sectors used in a FileSystem and stores the result in the Partition's FileSystem object.
    @param p the Partition the FileSystem is on
    @param mountPoint mount point of the partition in question
*/
static void readSectorsUsed(const Device& d, Partition& p, const QString& mountPoint)
{
    const KDiskFreeSpaceInfo freeSpaceInfo = KDiskFreeSpaceInfo::freeSpaceInfo(mountPoint);

    if (p.isMounted() && freeSpaceInfo.isValid() && mountPoint != QString())
        p.fileSystem().setSectorsUsed(freeSpaceInfo.used() / d.logicalSectorSize());
    else if (p.fileSystem().supportGetUsed() == FileSystem::cmdSupportFileSystem)
        p.fileSystem().setSectorsUsed(p.fileSystem().readUsedCapacity(p.deviceNode()) / d.logicalSectorSize());
#if defined LIBPARTED_FS_RESIZE_LIBRARY_SUPPORT
    else if (p.fileSystem().supportGetUsed() == FileSystem::cmdSupportBackend)
        p.fileSystem().setSectorsUsed(readSectorsUsedLibParted(p));
#endif
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

/** Create a Device for the given deviceNode and scan it for partitions.
    @param deviceNode the device node (e.g. "/dev/sda")
    @return the created Device object. callers need to free this.
*/
Device* LibPartedBackend::scanDevice(const QString& deviceNode)
{
    QVariantMap args;
    args[QLatin1String("deviceNode")] = deviceNode;

    KAuth::Action scanAction = QStringLiteral("org.kde.kpmcore.scan.scandevice");
    scanAction.setHelperId(QStringLiteral("org.kde.kpmcore.scan"));
    scanAction.setArguments(args);
    KAuth::ExecuteJob *job = scanAction.execute();
    if (!job->exec()) {
        qWarning() << "KAuth returned an error code: " << job->errorString();
        return nullptr;
    }

    bool pedDeviceError = job->data()[QLatin1String("pedDeviceError")].toBool();

    if (pedDeviceError) {
        Log(Log::warning) << xi18nc("@info:status", "Could not access device <filename>%1</filename>", deviceNode);
        return nullptr;
    }

    QString model = job->data()[QLatin1String("model")].toString();
    QString path = job->data()[QLatin1String("path")].toString();
    int heads = job->data()[QLatin1String("heads")].toInt();
    int sectors = job->data()[QLatin1String("sectors")].toInt();
    int cylinders = job->data()[QLatin1String("cylinders")].toInt();
    int sectorSize = job->data()[QLatin1String("sectorSize")].toInt();
    bool pedDiskError = job->data()[QLatin1String("pedDiskError")].toBool();

    Log(Log::information) << xi18nc("@info:status", "Device found: %1", model);

    Device* d = new Device(model, path, heads, sectors, cylinders, sectorSize);

    if (pedDiskError)
        return d;

    QString typeName = job->data()[QLatin1String("typeName")].toString();
    qint32 maxPrimaryPartitionCount = job->data()[QLatin1String("maxPrimaryPartitionCount")].toInt();
    quint64 firstUsableSector = job->data()[QLatin1String("firstUsableSector")].toULongLong();
    quint64 lastUsableSector = job->data()[QLatin1String("lastUsableSector")].toULongLong();

    const PartitionTable::TableType type = PartitionTable::nameToTableType(typeName);
    CoreBackend::setPartitionTableForDevice(*d, new PartitionTable(type, firstUsableSector, lastUsableSector));
    CoreBackend::setPartitionTableMaxPrimaries(*d->partitionTable(), maxPrimaryPartitionCount);

    KMountPoint::List mountPoints = KMountPoint::currentMountPoints(KMountPoint::NeedRealDeviceName);
    mountPoints.append(KMountPoint::possibleMountPoints(KMountPoint::NeedRealDeviceName));

    QList<QVariant> partitionPath = job->data()[QLatin1String("partitionPath")].toList();
    QList<QVariant> partitionType = job->data()[QLatin1String("partitionType")].toList();
    QList<QVariant> partitionStart = job->data()[QLatin1String("partitionStart")].toList();
    QList<QVariant> partitionEnd = job->data()[QLatin1String("partitionEnd")].toList();
    QList<QVariant> partitionBusy = job->data()[QLatin1String("partitionBusy")].toList();

    quint32 totalPartitions = partitionPath.size();
    QList<Partition*> partitions;
    for (quint32 i = 0; i < totalPartitions; ++i) {
        QString partitionNode = partitionPath[i].toString();
        int type = partitionType[i].toInt();
        qint64 start = partitionStart[i].toLongLong();
        qint64 end = partitionEnd[i].toLongLong();
        bool busy = partitionBusy[i].toBool();

        PartitionRole::Roles r = PartitionRole::None;

        FileSystem::Type fsType = detectFileSystem(partitionNode);

        switch (type) {
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
        PartitionNode* parent = d->partitionTable()->findPartitionBySector(start, PartitionRole(PartitionRole::Extended));

        // None found, so it's a primary in the device's partition table.
        if (parent == nullptr)
            parent = d->partitionTable();

        FileSystem* fs = FileSystemFactory::create(fsType, start, end);

        // libparted does not handle LUKS partitions
        QString mountPoint;
        bool mounted = false;
        if (fsType == FileSystem::Luks) {
            r |= PartitionRole::Luks;
            FS::luks* luksFs = dynamic_cast<FS::luks*>(fs);
            QString mapperNode = FS::luks::mapperName(partitionNode);
            bool isCryptOpen = !mapperNode.isEmpty();
            luksFs->setCryptOpen(isCryptOpen);
            luksFs->setLogicalSectorSize(d->logicalSectorSize());

            if (isCryptOpen) {
                luksFs->loadInnerFileSystem(partitionNode, mapperNode);

                mountPoint = mountPoints.findByDevice(mapperNode) ?
                             mountPoints.findByDevice(mapperNode)->mountPoint() :
                             QString();
                // We cannot use libparted to check the mounted status because
                // we don't have a PedPartition for the mapper device, so we use lsblk
                mounted = isMounted(mapperNode);
                if (mounted) {
                    const KDiskFreeSpaceInfo freeSpaceInfo = KDiskFreeSpaceInfo::freeSpaceInfo(mountPoint);
                    if (freeSpaceInfo.isValid() && mountPoint != QString())
                        luksFs->setSectorsUsed(freeSpaceInfo.used() / d->logicalSectorSize() + luksFs->getPayloadOffset(partitionNode));
                }
            } else {
                mounted = false;
            }

            luksFs->setMounted(mounted);
        } else {
            mountPoint = mountPoints.findByDevice(partitionNode) ?
                         mountPoints.findByDevice(partitionNode)->mountPoint() :
                         QString();
            mounted = busy;
        }

        QList<QVariant> availableFlags = job->data()[QLatin1String("availableFlags")].toList();
        PartitionTable::Flags available = static_cast<PartitionTable::Flag>(availableFlags[i].toInt());
        QList<QVariant> activeFlags = job->data()[QLatin1String("activeFlags")].toList();
        PartitionTable::Flags active = static_cast<PartitionTable::Flag>(activeFlags[i].toInt());
        Partition* part = new Partition(parent, *d, PartitionRole(r), fs, start, end, partitionNode, available, mountPoint, mounted, active);

        if (!part->roles().has(PartitionRole::Luks))
            readSectorsUsed(*d, *part, mountPoint);

        if (fs->supportGetLabel() != FileSystem::cmdSupportNone)
            fs->setLabel(fs->readLabel(part->deviceNode()));

        if (fs->supportGetUUID() != FileSystem::cmdSupportNone)
            fs->setUUID(fs->readUUID(part->deviceNode()));

        parent->append(part);
        partitions.append(part);
    }

    d->partitionTable()->updateUnallocated(*d);

    if (d->partitionTable()->isSectorBased(*d))
        d->partitionTable()->setType(*d, PartitionTable::msdos_sectorbased);

    foreach(const Partition * part, partitions)
        PartitionAlignment::isAligned(*d, *part);


    return d;
}

QList<Device*> LibPartedBackend::scanDevices(bool excludeReadOnly)
{
    QList<Device*> result;
    // FIXME: cat /sys/block/loop0/ro
    // linux.git/tree/Documentation/devices.txt
    QString blockDeviceMajorNumbers = QStringLiteral(
        "3,22,33,34,56,57,88,89,90,91,128,129,130,131,132,133,134,135," // MFM, RLL and IDE hard disk/CD-ROM interface
        "7," // loop devices
        "8,65,66,67,68,69,70,71," // SCSI disk devices
        "80,81,82,83,84,85,86,87," // I2O hard disk
        "179," // MMC block devices
        "259" // Block Extended Major (include NVMe)
    );
    ExternalCommand cmd(QStringLiteral("lsblk"), {
                          QStringLiteral("--nodeps"),
                          QStringLiteral("--noheadings"),
                          QStringLiteral("--output"), QString::fromLatin1("name"),
                          QStringLiteral("--paths"),
                          QStringLiteral("--include"), blockDeviceMajorNumbers});
    if (cmd.run(-1) && cmd.exitCode() == 0) {
        QStringList devices = cmd.output().split(QString::fromLatin1("\n"));
        devices.removeLast();
        quint32 totalDevices = devices.length();
        for (quint32 i = 0; i < totalDevices; ++i) {
            if (excludeReadOnly) {
                QFile f(QStringLiteral("/sys/block/%1/ro").arg(QString(devices[i]).remove(QStringLiteral("/dev/"))));
                if (f.open(QIODevice::ReadOnly))
                    if (f.readLine().trimmed().toInt() == 1)
                        continue;
            }

            emitScanProgress(devices[i], i * 100 / totalDevices);
            result.append(scanDevice(devices[i]));
        }
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

    ExternalCommand cmd(QString::fromLatin1("udevadm"),
                     { QString::fromLatin1("info"),
                       QString::fromLatin1("--query"),
                       QString::fromLatin1("property"),
                       partitionPath } );
    QRegularExpression re(QString::fromLatin1("(ID_FS_TYPE=)([\\w]+)"));

    if (!cmd.run(-1) || !(cmd.exitCode() == 0))
        return rval;
    QRegularExpressionMatch reFileSystemType = re.match(cmd.output());
    if (!reFileSystemType.hasMatch())
        return rval;

    QString s = reFileSystemType.captured(2);

    if (s == QString::fromLatin1("ext2")) rval = FileSystem::Ext2;
    else if (s == QString::fromLatin1("ext3")) rval = FileSystem::Ext3;
    else if (s.startsWith(QString::fromLatin1("ext4"))) rval = FileSystem::Ext4;
    else if (s == QString::fromLatin1("swap")) rval = FileSystem::LinuxSwap;
    else if (s == QString::fromLatin1("ntfs")) rval = FileSystem::Ntfs;
    else if (s == QString::fromLatin1("reiserfs")) rval = FileSystem::ReiserFS;
    else if (s == QString::fromLatin1("reiser4")) rval = FileSystem::Reiser4;
    else if (s == QString::fromLatin1("xfs")) rval = FileSystem::Xfs;
    else if (s == QString::fromLatin1("jfs")) rval = FileSystem::Jfs;
    else if (s == QString::fromLatin1("hfs")) rval = FileSystem::Hfs;
    else if (s == QString::fromLatin1("hfsplus")) rval = FileSystem::HfsPlus;
    else if (s == QString::fromLatin1("ufs")) rval = FileSystem::Ufs;
    else if (s == QString::fromLatin1("vfat")) {
    // udev uses ID_FS_VERSION to distinguish between FAT12, FAT16 and FAT32
        re.setPattern(QString::fromLatin1("(ID_FS_VERSION=)([\\w]+)"));
        QRegularExpressionMatch reFileSystemType = re.match(cmd.output());
        if (!reFileSystemType.hasMatch())
            return rval;
        QString fsVersion = reFileSystemType.captured(2);
        if (fsVersion  == QString::fromLatin1("FAT16"))
            rval = FileSystem::Fat16;
        if (fsVersion  == QString::fromLatin1("FAT32"))
            rval = FileSystem::Fat32;
        else
            rval = FileSystem::Fat16; // FIXME FAT12
    } else if (s == QString::fromLatin1("btrfs")) rval = FileSystem::Btrfs;
    else if (s == QString::fromLatin1("ocfs2")) rval = FileSystem::Ocfs2;
    else if (s == QString::fromLatin1("zfs_member")) rval = FileSystem::Zfs;
    else if (s == QString::fromLatin1("hpfs")) rval = FileSystem::Hpfs;
    else if (s == QString::fromLatin1("crypto_LUKS")) rval = FileSystem::Luks;
    else if (s == QString::fromLatin1("exfat")) rval = FileSystem::Exfat;
    else if (s == QString::fromLatin1("nilfs2")) rval = FileSystem::Nilfs2;
    else if (s == QString::fromLatin1("LVM2_member")) rval = FileSystem::Lvm2_PV;
    else if (s == QString::fromLatin1("f2fs")) rval = FileSystem::F2fs;
    else
        qWarning() << "udev: unknown file system type " << s << " on " << partitionPath;

    return rval;
}

CoreBackendDevice* LibPartedBackend::openDevice(const QString& deviceNode)
{
    LibPartedDevice* device = new LibPartedDevice(deviceNode);

    if (device == nullptr || !device->open()) {
        delete device;
        device = nullptr;
    }

    return device;
}

CoreBackendDevice* LibPartedBackend::openDeviceExclusive(const QString& deviceNode)
{
    LibPartedDevice* device = new LibPartedDevice(deviceNode);

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
    for (quint32 i = 0; i < sizeof(flagmap) / sizeof(flagmap[0]); i++)
        if (flagmap[i].flag == flag)
            return flagmap[i].pedFlag;

    return static_cast<PedPartitionFlag>(-1);
}

QString LibPartedBackend::lastPartedExceptionMessage()
{
    return s_lastPartedExceptionMessage;
}

#include "libpartedbackend.moc"
