/*************************************************************************
  *  Copyright (C) 2017 by Andrius Štikonas <andrius@stikonas.eu>        *
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

#include "plugins/sfdisk/sfdiskbackend.h"
#include "plugins/sfdisk/sfdiskdevice.h"

#include "core/copysourcedevice.h"
#include "core/copytargetbytearray.h"
#include "core/diskdevice.h"
#include "core/lvmdevice.h"
#include "core/partitiontable.h"
#include "core/partitionalignment.h"
#include "core/raid/softwareraid.h"

#include "fs/filesystemfactory.h"
#include "fs/luks.h"
#include "fs/luks2.h"

#include "util/globallog.h"
#include "util/externalcommand.h"
#include "util/helpers.h"

#include <QDataStream>
#include <QDebug>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QStorageInfo>
#include <QString>
#include <QStringList>

#include <KLocalizedString>
#include <KPluginFactory>

K_PLUGIN_FACTORY_WITH_JSON(SfdiskBackendFactory, "pmsfdiskbackendplugin.json", registerPlugin<SfdiskBackend>();)

SfdiskBackend::SfdiskBackend(QObject*, const QList<QVariant>&) :
    CoreBackend()
{
}

void SfdiskBackend::initFSSupport()
{
}

QList<Device*> SfdiskBackend::scanDevices(bool excludeReadOnly)
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
        const QJsonObject jsonObject = jsonDocument.object();
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
            if (device != nullptr) {
                result.append(device);
            }
        }

        SoftwareRAID::scanSoftwareRAID(result);
        LvmDevice::scanSystemLVM(result); // LVM scanner needs all other devices, so should be last
    }

    return result;
}

/** Create a Device for the given device_node and scan it for partitions.
    @param deviceNode the device node (e.g. "/dev/sda")
    @return the created Device object. callers need to free this.
*/
Device* SfdiskBackend::scanDevice(const QString& deviceNode)
{
    ExternalCommand modelCommand(QStringLiteral("lsblk"),
                        { QStringLiteral("--nodeps"),
                          QStringLiteral("--noheadings"),
                          QStringLiteral("--output"), QStringLiteral("model"),
                          deviceNode });
    ExternalCommand sizeCommand(QStringLiteral("blockdev"), { QStringLiteral("--getsize64"), deviceNode });
    ExternalCommand sizeCommand2(QStringLiteral("blockdev"), { QStringLiteral("--getss"), deviceNode });
    ExternalCommand jsonCommand(QStringLiteral("sfdisk"), { QStringLiteral("--json"), deviceNode }, QProcess::ProcessChannelMode::SeparateChannels );

    if ( sizeCommand.run(-1) && sizeCommand.exitCode() == 0
         && sizeCommand2.run(-1) && sizeCommand2.exitCode() == 0
         && jsonCommand.run(-1) )
    {
        Device* d = nullptr;
        qint64 deviceSize = sizeCommand.output().trimmed().toLongLong();
        int logicalSectorSize = sizeCommand2.output().trimmed().toLongLong();

        QFile mdstat(QStringLiteral("/proc/mdstat"));

        if (mdstat.open(QIODevice::ReadOnly)) {
            QTextStream stream(&mdstat);

            QString content = stream.readAll();

            mdstat.close();

            QRegularExpression re(QStringLiteral("md([\\/\\w]+)\\s+:"));
            QRegularExpressionMatchIterator i  = re.globalMatch(content);

            while (i.hasNext()) {

                QRegularExpressionMatch reMatch = i.next();

                QString name = reMatch.captured(1);

                if ((QStringLiteral("/dev/md") + name) == deviceNode) {
                    Log(Log::Level::information) << xi18nc("@info:status", "Software RAID Device found: %1", deviceNode);

                    d = new SoftwareRAID( QStringLiteral("md") + name, SoftwareRAID::Status::Active );

                    break;
                }

            }
        }

        if ( d == nullptr && modelCommand.run(-1) && modelCommand.exitCode() == 0 )
        {
            QString name = modelCommand.output();
            name = name.left(name.length() - 1);

            if (name.trimmed().isEmpty()) {
                // Get 'lsblk --output kname' in the cases where the model name is not available.
                // As lsblk doesn't have an option to include a separator in its output, it is
                // necessary to run it again getting only the kname as output.
                ExternalCommand kname(QStringLiteral("lsblk"), {QStringLiteral("--nodeps"), QStringLiteral("--noheadings"), QStringLiteral("--output"), QStringLiteral("kname"),
                                                                deviceNode});

                if (kname.run(-1) && kname.exitCode() == 0)
                    name = kname.output();
            }

            Log(Log::Level::information) << xi18nc("@info:status", "Device found: %1", name);

            d = new DiskDevice(name, deviceNode, 255, 63, deviceSize / logicalSectorSize / 255 / 63, logicalSectorSize);
        }

        if ( d )
        {
            if (jsonCommand.exitCode() != 0)
                return d;

            const QJsonObject jsonObject = QJsonDocument::fromJson(jsonCommand.rawOutput()).object();
            const QJsonObject partitionTable = jsonObject[QLatin1String("partitiontable")].toObject();

            if (!updateDevicePartitionTable(*d, partitionTable))
                return nullptr;

            return d;
        }
    }
    else
    {
        // Look if this device is a LVM VG
        ExternalCommand checkVG(QStringLiteral("lvm"), { QStringLiteral("vgdisplay"), deviceNode });

        if (checkVG.run(-1) && checkVG.exitCode() == 0)
        {
            QList<Device *> availableDevices = scanDevices();

            LvmDevice::scanSystemLVM(availableDevices);

            for (Device *device : qAsConst(availableDevices))
                if (device->deviceNode() == deviceNode)
                    return device;
        }
    }
    return nullptr;
}

/** Scans a Device for Partitions.

    This method  will scan a Device for all Partitions on it, detect the FileSystem for each Partition,
    try to determine the FileSystem usage, read the FileSystem label and store it all in newly created
    objects that are in the end added to the Device's PartitionTable.
*/
void SfdiskBackend::scanDevicePartitions(Device& d, const QJsonArray& jsonPartitions)
{
    Q_ASSERT(d.partitionTable());

    QList<Partition*> partitions;
    for (const auto &partition : jsonPartitions) {
        const QJsonObject partitionObject = partition.toObject();
        const QString partitionNode = partitionObject[QLatin1String("node")].toString();
        const qint64 start = partitionObject[QLatin1String("start")].toVariant().toLongLong();
        const qint64 size = partitionObject[QLatin1String("size")].toVariant().toLongLong();
        const QString partitionType = partitionObject[QLatin1String("type")].toString();
        PartitionTable::Flags activeFlags = partitionObject[QLatin1String("bootable")].toBool() ? PartitionTable::FlagBoot : PartitionTable::FlagNone;

        if (partitionType == QStringLiteral("C12A7328-F81F-11D2-BA4B-00A0C93EC93B"))
            activeFlags |= PartitionTable::FlagBoot;
        else if (partitionType == QStringLiteral("21686148-6449-6E6F-744E-656564454649"))
            activeFlags |= PartitionTable::FlagBiosGrub;

        FileSystem::Type type = FileSystem::Type::Unknown;
        type = detectFileSystem(partitionNode);
        PartitionRole::Roles r = PartitionRole::Primary;

        if ( (d.partitionTable()->type() == PartitionTable::msdos || d.partitionTable()->type() == PartitionTable::msdos_sectorbased) && partitionType.toInt() == 5 ) {
            r = PartitionRole::Extended;
            type = FileSystem::Type::Extended;
        }

        // Find an extended partition this partition is in.
        PartitionNode* parent = d.partitionTable()->findPartitionBySector(start, PartitionRole(PartitionRole::Extended));

        // None found, so it's a primary in the device's partition table.
        if (parent == nullptr)
            parent = d.partitionTable();
        else
            r = PartitionRole::Logical;

        FileSystem* fs = FileSystemFactory::create(type, start, start + size - 1, d.logicalSize());
        fs->scan(partitionNode);

        QString mountPoint;
        bool mounted;
        // sfdisk does not handle LUKS partitions
        if (fs->type() == FileSystem::Type::Luks || fs->type() == FileSystem::Type::Luks2) {
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

        Partition* part = new Partition(parent, d, PartitionRole(r), fs, start, start + size - 1, partitionNode, availableFlags(d.partitionTable()->type()), mountPoint, mounted, activeFlags);

        if (!part->roles().has(PartitionRole::Luks))
            readSectorsUsed(d, *part, mountPoint);

        if (fs->supportGetLabel() != FileSystem::cmdSupportNone)
            fs->setLabel(fs->readLabel(part->deviceNode()));

        if (d.partitionTable()->type() == PartitionTable::TableType::gpt) {
            part->setLabel(partitionObject[QLatin1String("name")].toString());
            part->setUUID(partitionObject[QLatin1String("uuid")].toString());
        }

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

bool SfdiskBackend::updateDevicePartitionTable(Device &d, const QJsonObject &jsonPartitionTable)
{
    QString tableType = jsonPartitionTable[QLatin1String("label")].toString();
    const PartitionTable::TableType type = PartitionTable::nameToTableType(tableType);

    qint64 firstUsableSector = 0, lastUsableSector;

    if ( d.type() == Device::Type::Disk_Device )
    {
        const DiskDevice* diskDevice = static_cast<const DiskDevice*>(&d);

        lastUsableSector = diskDevice->totalSectors();
    }
    else if ( d.type() == Device::Type::SoftwareRAID_Device )
    {
        const SoftwareRAID* raidDevice = static_cast<const SoftwareRAID*>(&d);

        lastUsableSector = raidDevice->totalLogical() - 1;
    }

    if (type == PartitionTable::gpt) {
        firstUsableSector = jsonPartitionTable[QLatin1String("firstlba")].toVariant().toLongLong();
        lastUsableSector = jsonPartitionTable[QLatin1String("lastlba")].toVariant().toLongLong();
    }

    if (lastUsableSector < firstUsableSector) {
        return false;
    }

    setPartitionTableForDevice(d, new PartitionTable(type, firstUsableSector, lastUsableSector));
    switch (type) {
    case PartitionTable::gpt:
    {
        // Read the maximum number of GPT partitions
        qint32 maxEntries;
        QByteArray gptHeader;
        CopySourceDevice source(d, 512, 1023);
        CopyTargetByteArray target(gptHeader);

        ExternalCommand copyCmd;
        if (copyCmd.copyBlocks(source, target)) {
            QByteArray gptMaxEntries = gptHeader.mid(80, 4);
            QDataStream stream(&gptMaxEntries, QIODevice::ReadOnly);
            stream.setByteOrder(QDataStream::LittleEndian);
            stream >> maxEntries;
        }
        else
            maxEntries = 128;
        CoreBackend::setPartitionTableMaxPrimaries(*d.partitionTable(), maxEntries);
    }
    default:
        break;
    }

    scanDevicePartitions(d, jsonPartitionTable[QLatin1String("partitions")].toArray());

    return true;
}

/** Reads the sectors used in a FileSystem and stores the result in the Partition's FileSystem object.
    @param p the Partition the FileSystem is on
    @param mountPoint mount point of the partition in question
*/
void SfdiskBackend::readSectorsUsed(const Device& d, Partition& p, const QString& mountPoint)
{
    if (!mountPoint.isEmpty() && p.fileSystem().type() != FileSystem::Type::LinuxSwap && p.fileSystem().type() != FileSystem::Type::Lvm2_PV) {
        const QStorageInfo storage = QStorageInfo(mountPoint);
        if (p.isMounted() && storage.isValid())
            p.fileSystem().setSectorsUsed( (storage.bytesTotal() - storage.bytesFree()) / d.logicalSize());
    }
    else if (p.fileSystem().supportGetUsed() == FileSystem::cmdSupportFileSystem)
        p.fileSystem().setSectorsUsed(p.fileSystem().readUsedCapacity(p.deviceNode()) / d.logicalSize());
}

FileSystem::Type SfdiskBackend::detectFileSystem(const QString& partitionPath)
{
    FileSystem::Type rval = FileSystem::Type::Unknown;

    ExternalCommand udevCommand(QStringLiteral("udevadm"), {
                                 QStringLiteral("info"),
                                 QStringLiteral("--query=property"),
                                 partitionPath });

    if (udevCommand.run(-1) && udevCommand.exitCode() == 0) {
        QRegularExpression re(QStringLiteral("ID_FS_TYPE=(\\w+)"));
        QRegularExpression re2(QStringLiteral("ID_FS_VERSION=(\\w+)"));
        QRegularExpressionMatch reFileSystemType = re.match(udevCommand.output());
        QRegularExpressionMatch reFileSystemVersion = re2.match(udevCommand.output());

        QString s;
        if (reFileSystemType.hasMatch()) {
            s = reFileSystemType.captured(1);
        }

        QString version;
        if (reFileSystemVersion.hasMatch()) {
            version = reFileSystemVersion.captured(1);
        }

        if (s == QStringLiteral("ext2")) rval = FileSystem::Type::Ext2;
        else if (s == QStringLiteral("ext3")) rval = FileSystem::Type::Ext3;
        else if (s.startsWith(QStringLiteral("ext4"))) rval = FileSystem::Type::Ext4;
        else if (s == QStringLiteral("swap")) rval = FileSystem::Type::LinuxSwap;
        else if (s == QStringLiteral("ntfs")) rval = FileSystem::Type::Ntfs;
        else if (s == QStringLiteral("reiserfs")) rval = FileSystem::Type::ReiserFS;
        else if (s == QStringLiteral("reiser4")) rval = FileSystem::Type::Reiser4;
        else if (s == QStringLiteral("xfs")) rval = FileSystem::Type::Xfs;
        else if (s == QStringLiteral("jfs")) rval = FileSystem::Type::Jfs;
        else if (s == QStringLiteral("hfs")) rval = FileSystem::Type::Hfs;
        else if (s == QStringLiteral("hfsplus")) rval = FileSystem::Type::HfsPlus;
        else if (s == QStringLiteral("ufs")) rval = FileSystem::Type::Ufs;
        else if (s == QStringLiteral("vfat")) {
            if (version == QStringLiteral("FAT32"))
                rval = FileSystem::Type::Fat32;
            else if (version == QStringLiteral("FAT16"))
                rval = FileSystem::Type::Fat16;
            else if (version == QStringLiteral("FAT12"))
                rval = FileSystem::Type::Fat12;
        }
        else if (s == QStringLiteral("btrfs")) rval = FileSystem::Type::Btrfs;
        else if (s == QStringLiteral("ocfs2")) rval = FileSystem::Type::Ocfs2;
        else if (s == QStringLiteral("zfs_member")) rval = FileSystem::Type::Zfs;
        else if (s == QStringLiteral("hpfs")) rval = FileSystem::Type::Hpfs;
        else if (s == QStringLiteral("crypto_LUKS")) {
            if (version == QStringLiteral("1"))
                rval = FileSystem::Type::Luks;
            else if (version == QStringLiteral("2")) {
                rval = FileSystem::Type::Luks2;
            }
        }
        else if (s == QStringLiteral("exfat")) rval = FileSystem::Type::Exfat;
        else if (s == QStringLiteral("nilfs2")) rval = FileSystem::Type::Nilfs2;
        else if (s == QStringLiteral("LVM2_member")) rval = FileSystem::Type::Lvm2_PV;
        else if (s == QStringLiteral("f2fs")) rval = FileSystem::Type::F2fs;
        else if (s == QStringLiteral("udf")) rval = FileSystem::Type::Udf;
        else if (s == QStringLiteral("iso9660")) rval = FileSystem::Type::Iso9660;
        else if (s == QStringLiteral("linux_raid_member")) rval = FileSystem::Type::LinuxRaidMember;
        else
            qWarning() << "unknown file system type " << s << " on " << partitionPath;
    }

    return rval;
}

QString SfdiskBackend::readLabel(const QString& deviceNode) const
{
    ExternalCommand udevCommand(QStringLiteral("udevadm"), {
                                 QStringLiteral("info"),
                                 QStringLiteral("--query=property"),
                                 deviceNode });
    udevCommand.run();
    QRegularExpression re(QStringLiteral("ID_FS_LABEL=(.*)"));
    QRegularExpressionMatch reFileSystemLabel = re.match(udevCommand.output());
    if (reFileSystemLabel.hasMatch())
        return reFileSystemLabel.captured(1);

    return QString();
}

QString SfdiskBackend::readUUID(const QString& deviceNode) const
{
    ExternalCommand udevCommand(QStringLiteral("udevadm"), {
                                 QStringLiteral("info"),
                                 QStringLiteral("--query=property"),
                                 deviceNode });
    udevCommand.run();
    QRegularExpression re(QStringLiteral("ID_FS_UUID=(.*)"));
    QRegularExpressionMatch reFileSystemUUID = re.match(udevCommand.output());
    if (reFileSystemUUID.hasMatch())
        return reFileSystemUUID.captured(1);

    return QString();
}

PartitionTable::Flags SfdiskBackend::availableFlags(PartitionTable::TableType type)
{
    PartitionTable::Flags flags;
    if (type == PartitionTable::gpt) {
        // These are not really flags but for now keep them for compatibility
        // We should implement changing partition type
        flags = PartitionTable::Flag::FlagBiosGrub |
                PartitionTable::Flag::FlagBoot;
    }
    else if (type == PartitionTable::msdos || type == PartitionTable::msdos_sectorbased)
        flags = PartitionTable::FlagBoot;

    return flags;
}

std::unique_ptr<CoreBackendDevice> SfdiskBackend::openDevice(const Device& d)
{
    std::unique_ptr<SfdiskDevice> device = std::make_unique<SfdiskDevice>(d);

    if (!device->open())
        device = nullptr;

    return device;
}

std::unique_ptr<CoreBackendDevice> SfdiskBackend::openDeviceExclusive(const Device& d)
{
    std::unique_ptr<SfdiskDevice> device = std::make_unique<SfdiskDevice>(d);

    if (!device->openExclusive())
        device = nullptr;

    return device;
}

bool SfdiskBackend::closeDevice(std::unique_ptr<CoreBackendDevice> coreDevice)
{
    return coreDevice->close();
}

#include "sfdiskbackend.moc"
