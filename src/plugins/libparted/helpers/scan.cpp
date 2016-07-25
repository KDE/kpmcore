/*************************************************************************
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

#include "scan.h"
#include "core/partitiontable.h"
#include "plugins/libparted/pedflags.h"

#include <blkid/blkid.h>

ActionReply Scan::scandevice(const QVariantMap& args)
{
    ActionReply reply;

    QString deviceNode = args[QLatin1String("deviceNode")].toString();
    PedDevice* pedDevice = ped_device_get(deviceNode.toLocal8Bit().constData());

    QVariantMap returnArgs;
    if (!pedDevice) {
        returnArgs[QLatin1String("pedDeviceError")] = true;
        reply.setData(returnArgs);
        return reply;
    }

    returnArgs[QLatin1String("model")] = QString::fromUtf8(pedDevice->model);
    returnArgs[QLatin1String("path")] = QString::fromUtf8(pedDevice->path);
    returnArgs[QLatin1String("heads")] = pedDevice->bios_geom.heads;
    returnArgs[QLatin1String("sectors")] = pedDevice->bios_geom.sectors;
    returnArgs[QLatin1String("cylinders")] = pedDevice->bios_geom.cylinders;
    returnArgs[QLatin1String("sectorSize")] = pedDevice->sector_size;

    PedDisk* pedDisk = ped_disk_new(pedDevice);

    if (!pedDisk) {
        returnArgs[QLatin1String("pedDiskError")] = true;
        reply.setData(returnArgs);
        return reply;
    }

    quint64 firstUsableSector = pedDisk->dev->bios_geom.sectors;

    if (strcmp(pedDisk->type->name, "gpt") == 0) {
        GPTDiskData* gpt_disk_data = reinterpret_cast<GPTDiskData*>(pedDisk->disk_specific);
        PedGeometry* geom = reinterpret_cast<PedGeometry*>(&gpt_disk_data->data_area);

        if (geom)
            firstUsableSector = geom->start;
        else
            firstUsableSector += 32;
    }

    quint64 lastUsableSector = 0;
    lastUsableSector = static_cast< quint64 >( pedDisk->dev->bios_geom.sectors ) *
           pedDisk->dev->bios_geom.heads *
           pedDisk->dev->bios_geom.cylinders - 1;

    if (strcmp(pedDisk->type->name, "gpt") == 0) {
        GPTDiskData* gpt_disk_data = reinterpret_cast<GPTDiskData*>(pedDisk->disk_specific);
        PedGeometry* geom = reinterpret_cast<PedGeometry*>(&gpt_disk_data->data_area);

        if (geom)
            lastUsableSector = geom->end;
        else
            lastUsableSector -= 32;
    }

    returnArgs[QLatin1String("pedDeviceError")] = false;
    returnArgs[QLatin1String("pedDiskError")] = false;

    returnArgs[QLatin1String("typeName")] = QString::fromUtf8(pedDisk->type->name);
    returnArgs[QLatin1String("maxPrimaryPartitionCount")] = ped_disk_get_max_primary_partition_count(pedDisk);
    returnArgs[QLatin1String("firstUsableSector")] = firstUsableSector;
    returnArgs[QLatin1String("lastUsableSector")] = lastUsableSector;

    PedPartition* pedPartition = nullptr;
    QList<QVariant> partitionPath;
    QList<QVariant> partitionType;
    QList<QVariant> partitionStart;
    QList<QVariant> partitionEnd;
    QList<QVariant> partitionBusy;
    QList<QVariant> availableFlags;
    QList<QVariant> activeFlags;

    while ((pedPartition = ped_disk_next_partition(pedDisk, pedPartition))) {
        if (pedPartition->num < 1)
            continue;

        partitionPath.append(QLatin1String(ped_partition_get_path(pedPartition)));
        partitionType.append(pedPartition->type);
        partitionStart.append(pedPartition->geom.start);
        partitionEnd.append(pedPartition->geom.end);
        partitionBusy.append(ped_partition_is_busy(pedPartition));

        // --------------------------------------------------------------------------
        // Get list of available flags

        PartitionTable::Flags flags;

        // We might get here with a pedPartition just picked up from libparted that is
        // unallocated. Libparted doesn't like it if we ask for flags for unallocated
        // space.
        if (pedPartition->num > 0)
            for (quint32 i = 0; i < sizeof(flagmap) / sizeof(flagmap[0]); i++)
                if (ped_partition_is_flag_available(pedPartition, flagmap[i].pedFlag))
                    // Workaround: libparted claims the hidden flag is available for extended partitions, but
                    // throws an error when we try to set or clear it. So skip this combination. Also see setFlag.
                    if (pedPartition->type != PED_PARTITION_EXTENDED || flagmap[i].flag != PartitionTable::FlagHidden)
                        flags |= flagmap[i].flag;

        availableFlags.append(static_cast<qint32>(flags));
        // --------------------------------------------------------------------------
        // Get list of active flags

        flags = PartitionTable::FlagNone;
        if (pedPartition->num > 0)
            for (quint32 i = 0; i < sizeof(flagmap) / sizeof(flagmap[0]); i++)
                if (ped_partition_is_flag_available(pedPartition, flagmap[i].pedFlag) && ped_partition_get_flag(pedPartition, flagmap[i].pedFlag))
                    flags |= flagmap[i].flag;

        activeFlags.append(static_cast<qint32>(flags));
        // --------------------------------------------------------------------------
    }

    returnArgs[QLatin1String("availableFlags")] = availableFlags;
    returnArgs[QLatin1String("activeFlags")] = activeFlags;
    returnArgs[QLatin1String("partitionPath")] = partitionPath;
    returnArgs[QLatin1String("partitionType")] = partitionType;
    returnArgs[QLatin1String("partitionStart")] = partitionStart;
    returnArgs[QLatin1String("partitionEnd")] = partitionEnd;
    returnArgs[QLatin1String("partitionBusy")] = partitionBusy;

    reply.setData(returnArgs);
    return reply;
}

ActionReply Scan::readsectorsused(const QVariantMap& args)
{
    ActionReply reply;

    QString deviceNode = args[QLatin1String("deviceNode")].toString();
    qint64 firstSector = args[QLatin1String("firstSector")].toLongLong();
    qint64 rval = -1;

    if (PedDevice* pedDevice = ped_device_get(deviceNode.toLocal8Bit().constData()))
        if (PedDisk* pedDisk = ped_disk_new(pedDevice))
            if (PedPartition* pedPartition = ped_disk_get_partition_by_sector(pedDisk, firstSector))
                if (PedFileSystem* pedFileSystem = ped_file_system_open(&pedPartition->geom))
                    if (PedConstraint* pedConstraint = ped_file_system_get_resize_constraint(pedFileSystem))
                        rval = pedConstraint->min_size;

    QVariantMap returnArgs;
    returnArgs[QLatin1String("sectorsUsed")] = rval;

    reply.setData(returnArgs);
    return reply;
}

ActionReply Scan::detectfilesystem(const QVariantMap& args)
{
    ActionReply reply;
    QVariantMap returnArgs;
    QString deviceNode = args[QLatin1String("deviceNode")].toString();

    blkid_cache cache;
    if (blkid_get_cache(&cache, nullptr) == 0) {
        blkid_dev dev;

        if ((dev = blkid_get_dev(cache, deviceNode.toLocal8Bit().constData(), BLKID_DEV_NORMAL)) != nullptr) {
            char *string = blkid_get_tag_value(cache, "TYPE", deviceNode.toLocal8Bit().constData());
            QString s = QString::fromUtf8(string);
            free(string);

            if (s == QStringLiteral("vfat")) {
                // libblkid uses SEC_TYPE to distinguish between FAT16 and FAT32
                string = blkid_get_tag_value(cache, "SEC_TYPE", deviceNode.toLocal8Bit().constData());
                QString st = QString::fromUtf8(string);
                free(string);
                if (st == QStringLiteral("msdos"))
                    returnArgs[QLatin1String("fileSystem")] = QStringLiteral("fat16");
                else
                    returnArgs[QLatin1String("fileSystem")] = QStringLiteral("fat32");
            }
            else
                returnArgs[QLatin1String("fileSystem")] = s;
        }
        blkid_put_cache(cache);
    }

    reply.setData(returnArgs);
    return reply;
}

KAUTH_HELPER_MAIN("org.kde.kpmcore.scan", Scan)
