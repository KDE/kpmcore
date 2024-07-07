/*
    SPDX-FileCopyrightText: 2017-2020 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2018-2019 Caio Jordão Carvalho <caiojcarvalho@gmail.com>
    SPDX-FileCopyrightText: 2019 Shubham Jangra <aryan100jangid@gmail.com>
    SPDX-FileCopyrightText: 2020 Gaël PORTAY <gael.portay@collabora.com>
    SPDX-FileCopyrightText: 2020 Adriaan de Groot <groot@kde.org>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

/** @file
*/

#include "plugins/sfdisk/sfdiskbackend.h"
#include "plugins/sfdisk/sfdiskdevice.h"
#include "plugins/sfdisk/sfdiskgptattributes.h"

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

#include <utility>

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

K_PLUGIN_CLASS_WITH_JSON(SfdiskBackend, "pmsfdiskbackendplugin.json")

SfdiskBackend::SfdiskBackend(QObject*, const QList<QVariant>&) :
    CoreBackend()
{
}

void SfdiskBackend::initFSSupport()
{
}

QList<Device*> SfdiskBackend::scanDevices(const ScanFlags scanFlags)
{
    const bool includeReadOnly = scanFlags.testFlag(ScanFlag::includeReadOnly);
    const bool includeLoopback = scanFlags.testFlag(ScanFlag::includeLoopback);

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
            if (! (deviceObject[QLatin1String("type")].toString() == QLatin1String("disk")
                || (includeLoopback && deviceObject[QLatin1String("type")].toString() == QLatin1String("loop")) ))
            {
                continue;
            }

            const QString deviceNode = deviceObject[QLatin1String("name")].toString();
            if (!includeReadOnly) {
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
        
    }

    VolumeManagerDevice::scanDevices(result); // scan all types of VolumeManagerDevices

    return result;
}

/*** @brief Fix up bogus JSON from `sfdisk --json /dev/sdb`
 *
 * The command `sfdisk --json /dev/sdb` outputs a JSON representation
 * of the partition table, with general device characteristics and
 * the list of partitions, **but**..
 *
 * This isn't necessarily valid JSON: in particular, when there are
 * no partitions on the disk because it is empty / was recently zeroed /
 * is a USB stick for testing purposes, the output is changed **only**
 * by there being no partitions in the partition table. However,
 * the comma (",") after sectorsize is still printed. Bogus output looks
 * like this:
 *
 * {
 *   "partitiontable": {
 *       "label":"gpt",
 *       "id":"1F9E80D9-DD78-024F-94A3-B61EC82B18C8",
 *       "device":"/dev/sdb",
 *       "unit":"sectors",
 *       "firstlba":2048,
 *       "lastlba":30949342,
 *       "sectorsize":512,
 *   }
 * }
 *
 * That's not valid JSON because of the "," followed by nothing until
 * the brace, and yields an empty object is passed to fromJson().
 *
 * We'll go through and check if there's a "," followed by whitespace
 * and then a }. If there is, replace the ,.
 *
 * This is also fixed in util-linux 2.37.
 *
 * For some partition tables sfdisk prints an error message before the actual json
 * starts (seen with sfdisk 2.37.4). It looks like this:
 *
 * omitting empty partition (5)
 *  {
 *  "partitiontable": {
 *     "label": "dos",
 *     "id": "0x91769176",
 *     "device": "/dev/sdb",
 *     "unit": "sectors",
 *     "sectorsize": 512,
 *     "partitions": [
 *        {
 *           "node": "/dev/sdb1",
 *           "start": 63,
 *           "size": 84630357,
 *           "type": "7",
 *           "bootable": true
 *        },{
 * etc.
 */
static void
fixInvalidJsonFromSFDisk( QByteArray& s )
{
    int jsonStart = s.indexOf('{');
    if (jsonStart != 0) {
        const QByteArray invalidStart = s.left(jsonStart);
        qDebug() << "removed \"" << invalidStart.data() << "\" from beginning of sfdisk json output";
        const QByteArray copy = s.mid(jsonStart);
        s = copy;
    }

    // -1 if there is no comma (but then there's no useful JSON either),
    //    not is 0 a valid place (the start) for a , in a JSON document.
    int lastComma = s.lastIndexOf(',');
    if ( lastComma > 0 )
    {
        for ( int charIndex = lastComma + 1; charIndex < s.length(); ++charIndex )
        {
            if ( s[charIndex] == '}' )
            {
                s[lastComma] = ' ';  // Erase that comma
            }
            if ( !isspace( s[charIndex] ) )
            {
                break;
            }
        }
    }
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
    ExternalCommand sfdiskJsonCommand(QStringLiteral("sfdisk"), { QStringLiteral("--json"), deviceNode }, QProcess::ProcessChannelMode::SeparateChannels );

    if ( sizeCommand.run(-1) && sizeCommand.exitCode() == 0
         && sizeCommand2.run(-1) && sizeCommand2.exitCode() == 0
         && sfdiskJsonCommand.run(-1) )
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
            name = name.left(name.length() - 1).replace(QLatin1Char('_'), QLatin1Char(' '));

            if (name.trimmed().isEmpty()) {
                // Get 'lsblk --output kname' in the cases where the model name is not available.
                // As lsblk doesn't have an option to include a separator in its output, it is
                // necessary to run it again getting only the kname as output.
                ExternalCommand kname(QStringLiteral("lsblk"), {QStringLiteral("--nodeps"), QStringLiteral("--noheadings"), QStringLiteral("--output"), QStringLiteral("kname"),
                                                                deviceNode});

                if (kname.run(-1) && kname.exitCode() == 0)
                    name = kname.output().trimmed();
            }

            ExternalCommand transport(QStringLiteral("lsblk"), {QStringLiteral("--nodeps"), QStringLiteral("--noheadings"), QStringLiteral("--output"), QStringLiteral("tran"),
                                                                deviceNode});
            QString icon;
            if (transport.run(-1) && transport.exitCode() == 0)
                if (transport.output().trimmed() == QStringLiteral("usb"))
                    icon = QStringLiteral("drive-removable-media-usb");

            Log(Log::Level::information) << xi18nc("@info:status", "Device found: %1", name);

            d = new DiskDevice(name, deviceNode, logicalSectorSize, deviceSize / logicalSectorSize, icon);
        }

        if ( d )
        {
            if (sfdiskJsonCommand.exitCode() != 0) {
                scanWholeDevicePartition(*d);
                return d;
            }

            auto s = sfdiskJsonCommand.rawOutput();
            fixInvalidJsonFromSFDisk(s);

            const QJsonObject jsonObject = QJsonDocument::fromJson(s).object();
            const QJsonObject partitionTable = jsonObject[QLatin1String("partitiontable")].toObject();

            if (jsonObject.isEmpty()) {
                qDebug() << "json object created from sfdisk output is empty !\nOutput is \"" << s.data() << "\"";
            }

            /* Workaround for whole device FAT partitions */
            if(partitionTable[QLatin1String("label")].toString() == QStringLiteral("dos")) {
                scanWholeDevicePartition(*d);
                if(d->partitionTable()) {
                    return d;
                }
            }

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

            for (Device *device : std::as_const(availableDevices))
                if (device->deviceNode() == deviceNode)
                    return device;
        }
    }
    return nullptr;
}

/** Scans a Device for FileSystems spanning the whole block device

    This method  will scan a Device for a FileSystem.
    It tries to determine the FileSystem usage, reads the FileSystem label and creates
    PartitionTable of type "none" and a single Partition object.
*/
void SfdiskBackend::scanWholeDevicePartition(Device& d) {
    const QString partitionNode = d.deviceNode();
    constexpr qint64 firstSector = 0;
    const qint64 lastSector = d.totalLogical() - 1;
    setPartitionTableForDevice(d, new PartitionTable(PartitionTable::TableType::none, firstSector, lastSector));
    Partition *partition = scanPartition(d, partitionNode, firstSector, lastSector, QString(), false);

    if (partition->fileSystem().type() == FileSystem::Type::Unknown) {
        setPartitionTableForDevice(d, nullptr);
        delete d.partitionTable();
    }

    if (!partition->roles().has(PartitionRole::Luks))
        readSectorsUsed(d, *partition, partition->mountPoint());
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
        const bool bootable = partitionObject[QLatin1String("bootable")].toBool();
        const auto lastSector = start + size - 1;

        Partition* part = scanPartition(d, partitionNode, start, lastSector, partitionType, bootable);

        setupPartitionInfo(d, part, partitionObject);

        partitions.append(part);
    }

    d.partitionTable()->updateUnallocated(d);
    d.partitionTable()->setType(d, d.partitionTable()->type());

    for (const Partition *part : std::as_const(partitions))
        PartitionAlignment::isAligned(d, *part);
}

Partition* SfdiskBackend::scanPartition(Device& d, const QString& partitionNode, const qint64 firstSector, const qint64 lastSector, const QString& partitionType, const bool bootable)
{
    PartitionTable::Flags activeFlags = bootable ? PartitionTable::Flag::Boot : PartitionTable::Flag::None;
    if (partitionType == QStringLiteral("C12A7328-F81F-11D2-BA4B-00A0C93EC93B"))
        activeFlags |= PartitionTable::Flag::Boot;
    else if (partitionType == QStringLiteral("21686148-6449-6E6F-744E-656564454649"))
        activeFlags |= PartitionTable::Flag::BiosGrub;

    FileSystem::Type type = detectFileSystem(partitionNode);
    PartitionRole::Roles r = PartitionRole::Primary;

    if ( (d.partitionTable()->type() == PartitionTable::msdos) &&
        ( partitionType == QStringLiteral("5") || partitionType == QStringLiteral("f") ) ) {
        r = PartitionRole::Extended;
        type = FileSystem::Type::Extended;
    }

    // Find an extended partition this partition is in.
    PartitionNode* parent = d.partitionTable()->findPartitionBySector(firstSector, PartitionRole(PartitionRole::Extended));

    // None found, so it's a primary in the device's partition table.
    if (parent == nullptr)
        parent = d.partitionTable();
    else
        r = PartitionRole::Logical;

    FileSystem* fs = FileSystemFactory::create(type, firstSector, lastSector, d.logicalSize());
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

    Partition* partition = new Partition(parent, d, PartitionRole(r), fs, firstSector, lastSector, partitionNode, availableFlags(d.partitionTable()->type()), mountPoint, mounted, activeFlags);

    if (fs->supportGetLabel() != FileSystem::cmdSupportNone)
        fs->setLabel(fs->readLabel(partition->deviceNode()));

    if (fs->supportGetUUID() != FileSystem::cmdSupportNone)
        fs->setUUID(fs->readUUID(partition->deviceNode()));

    parent->append(partition);
    return partition;
}

void SfdiskBackend::setupPartitionInfo(const Device &d, Partition *partition, const QJsonObject& partitionObject)
{
    if (!partition->roles().has(PartitionRole::Luks))
        readSectorsUsed(d, *partition, partition->mountPoint());

    if (d.partitionTable()->type() == PartitionTable::TableType::gpt) {
        partition->setLabel(partitionObject[QLatin1String("name")].toString());
        partition->setUUID(partitionObject[QLatin1String("uuid")].toString());
        partition->setType(partitionObject[QLatin1String("type")].toString());
        QString attrs = partitionObject[QLatin1String("attrs")].toString();
        partition->setAttributes(SfdiskGptAttributes::toULongLong(attrs.split(QLatin1Char(' '))));
    }
}

bool SfdiskBackend::updateDevicePartitionTable(Device &d, const QJsonObject &jsonPartitionTable)
{
    QString tableType = jsonPartitionTable[QLatin1String("label")].toString();
    const PartitionTable::TableType type = PartitionTable::nameToTableType(tableType);

    qint64 firstUsableSector = 0;
    qint64 lastUsableSector = 0;

    if (d.type() == Device::Type::Disk_Device) {
        const DiskDevice* diskDevice = static_cast<const DiskDevice*>(&d);

        lastUsableSector = diskDevice->totalSectors();
    }
    else if (d.type() == Device::Type::SoftwareRAID_Device) {
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
        qint64 sectorSize = d.logicalSize();
        CopySourceDevice source(d, sectorSize, sectorSize * 2 - 1);

        ExternalCommand readCmd;
        gptHeader = readCmd.readData(source);
        if (gptHeader != QByteArray()) {
            QByteArray gptMaxEntries = gptHeader.mid(80, 4);
            QDataStream stream(&gptMaxEntries, QIODevice::ReadOnly);
            stream.setByteOrder(QDataStream::LittleEndian);
            stream >> maxEntries;
        }
        else
            maxEntries = 128;
        CoreBackend::setPartitionTableMaxPrimaries(*d.partitionTable(), maxEntries);
        break;
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

    QString typeRegExp = QStringLiteral("ID_FS_TYPE=(\\w+)");
    QString versionRegExp = QStringLiteral("ID_FS_VERSION=(\\w+)");

    QString name = {};

    rval = runDetectFileSystemCommand(udevCommand, typeRegExp, versionRegExp, name);

    // Fallback to blkid which has slightly worse detection but it works on whole block device filesystems.
    if (rval == FileSystem::Type::Unknown) {
        ExternalCommand blkidCommand(QStringLiteral("blkid"), { partitionPath });
        typeRegExp = QStringLiteral("TYPE=\"(\\w+)\"");
        versionRegExp = QStringLiteral("SEC_TYPE=\"(\\w+)\"");
        rval = runDetectFileSystemCommand(blkidCommand, typeRegExp, versionRegExp, name);
    }

    if (rval == FileSystem::Type::Unknown) {
        qWarning() << "unknown file system type " << name << " on " << partitionPath;
    }
    return rval;
}

FileSystem::Type SfdiskBackend::runDetectFileSystemCommand(ExternalCommand& command, QString& typeRegExp, QString& versionRegExp, QString& name)
{
    FileSystem::Type rval = FileSystem::Type::Unknown;

    if (command.run(-1) && command.exitCode() == 0) {
        QRegularExpression re(typeRegExp);
        QRegularExpression re2(versionRegExp);
        QRegularExpressionMatch reFileSystemType = re.match(command.output());
        QRegularExpressionMatch reFileSystemVersion = re2.match(command.output());

        if (reFileSystemType.hasMatch()) {
            name = reFileSystemType.captured(1);
        }

        QString version = {};
        if (reFileSystemVersion.hasMatch()) {
            version = reFileSystemVersion.captured(1);
        }
        rval = fileSystemNameToType(name, version);
    }
    return rval;
}

FileSystem::Type SfdiskBackend::fileSystemNameToType(const QString& name, const QString& version)
{
    FileSystem::Type rval = FileSystem::Type::Unknown;

    if (name == QStringLiteral("ext2")) rval = FileSystem::Type::Ext2;
    else if (name == QStringLiteral("ext3")) rval = FileSystem::Type::Ext3;
    else if (name.startsWith(QStringLiteral("ext4"))) rval = FileSystem::Type::Ext4;
    else if (name == QStringLiteral("swap")) rval = FileSystem::Type::LinuxSwap;
    else if (name == QStringLiteral("ntfs")) rval = FileSystem::Type::Ntfs;
    else if (name == QStringLiteral("reiserfs")) rval = FileSystem::Type::ReiserFS;
    else if (name == QStringLiteral("reiser4")) rval = FileSystem::Type::Reiser4;
    else if (name == QStringLiteral("xfs")) rval = FileSystem::Type::Xfs;
    else if (name == QStringLiteral("jfs")) rval = FileSystem::Type::Jfs;
    else if (name == QStringLiteral("hfs")) rval = FileSystem::Type::Hfs;
    else if (name == QStringLiteral("hfsplus")) rval = FileSystem::Type::HfsPlus;
    else if (name == QStringLiteral("ufs")) rval = FileSystem::Type::Ufs;
    else if (name == QStringLiteral("vfat")) {
        if (version == QStringLiteral("FAT32"))
            rval = FileSystem::Type::Fat32;
        else if (version == QStringLiteral("FAT16") || version == QStringLiteral("msdos")) // blkid uses msdos for both FAT16 and FAT12
            rval = FileSystem::Type::Fat16;
        else if (version == QStringLiteral("FAT12"))
            rval = FileSystem::Type::Fat12;
    }
    else if (name == QStringLiteral("btrfs")) rval = FileSystem::Type::Btrfs;
    else if (name == QStringLiteral("ocfs2")) rval = FileSystem::Type::Ocfs2;
    else if (name == QStringLiteral("zfs_member")) rval = FileSystem::Type::Zfs;
    else if (name == QStringLiteral("hpfs")) rval = FileSystem::Type::Hpfs;
    else if (name == QStringLiteral("crypto_LUKS")) {
        if (version == QStringLiteral("1"))
            rval = FileSystem::Type::Luks;
        else if (version == QStringLiteral("2")) {
            rval = FileSystem::Type::Luks2;
        }
    }
    else if (name == QStringLiteral("exfat")) rval = FileSystem::Type::Exfat;
    else if (name == QStringLiteral("nilfs2")) rval = FileSystem::Type::Nilfs2;
    else if (name == QStringLiteral("LVM2_member")) rval = FileSystem::Type::Lvm2_PV;
    else if (name == QStringLiteral("f2fs")) rval = FileSystem::Type::F2fs;
    else if (name == QStringLiteral("udf")) rval = FileSystem::Type::Udf;
    else if (name == QStringLiteral("iso9660")) rval = FileSystem::Type::Iso9660;
    else if (name == QStringLiteral("linux_raid_member")) rval = FileSystem::Type::LinuxRaidMember;
    else if (name == QStringLiteral("BitLocker")) rval = FileSystem::Type::BitLocker;
    else if (name == QStringLiteral("apfs")) rval = FileSystem::Type::Apfs;
    else if (name == QStringLiteral("minix")) rval = FileSystem::Type::Minix;
    else if (name == QStringLiteral("bcachefs")) rval = FileSystem::Type::Bcachefs;

    return rval;
}

// udev encodes the labels with ID_LABEL_FS_ENC which is done with
// blkid_encode_string(). Within this function some 1-byte utf-8
// characters not considered safe (e.g. '\' or ' ') are encoded as hex
// TODO: Qt6: get a more efficient implementation from Qt
static QString decodeFsEncString(const QString &str)
{
    QString decoded;
    decoded.reserve(str.size());
    int i = 0;
    while (i < str.size()) {
        if (i <= str.size() - 4) {    // we need at least four characters \xAB
            if (str.at(i) == QLatin1Char('\\') &&
                str.at(i+1) == QLatin1Char('x')) {
                bool bOk;
                const int code = str.mid(i+2, 2).toInt(&bOk, 16);
                if (bOk && code >= 0x20 && code < 0x80) {
                    decoded += QChar(code);
                    i += 4;
                    continue;
                }
            }
        }
        decoded += str.at(i);
        ++i;
    }
    return decoded;
}

QString SfdiskBackend::readLabel(const QString& deviceNode) const
{
    ExternalCommand udevCommand(QStringLiteral("udevadm"), {
                                 QStringLiteral("info"),
                                 QStringLiteral("--query=property"),
                                 deviceNode });
    udevCommand.run();
    QRegularExpression re(QStringLiteral("ID_FS_LABEL_ENC=(.*)"));
    QRegularExpressionMatch reFileSystemLabel = re.match(udevCommand.output());
    if (reFileSystemLabel.hasMatch()) {
        QString escapedLabel = reFileSystemLabel.captured(1);
        return decodeFsEncString(escapedLabel);
    }

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
        flags = PartitionTable::Flag::BiosGrub |
                PartitionTable::Flag::Boot;
    }
    else if (type == PartitionTable::msdos)
        flags = PartitionTable::Flag::Boot;

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
