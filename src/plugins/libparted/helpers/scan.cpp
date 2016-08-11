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

ActionReply Scan::scandevice(const QVariantMap& args)
{
    ActionReply reply;

    PedDevice* pedDevice = ped_device_get(args[QStringLiteral("deviceNode")].toString().toLocal8Bit().constData());

    if (!pedDevice) {
        reply.addData(QStringLiteral("pedDeviceError"), true);
        return reply;
    }

    reply.addData(QStringLiteral("model"), QString::fromUtf8(pedDevice->model));
    reply.addData(QStringLiteral("path"), QString::fromUtf8(pedDevice->path));
    reply.addData(QStringLiteral("heads"), pedDevice->bios_geom.heads);
    reply.addData(QStringLiteral("sectors"), pedDevice->bios_geom.sectors);
    reply.addData(QStringLiteral("cylinders"), pedDevice->bios_geom.cylinders);
    reply.addData(QStringLiteral("sectorSize"), pedDevice->sector_size);

    PedDisk* pedDisk = ped_disk_new(pedDevice);

    if (!pedDisk) {
        reply.addData(QStringLiteral("pedDiskError"), true);
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

    reply.addData(QStringLiteral("pedDeviceError"), false);
    reply.addData(QStringLiteral("pedDiskError"), false);

    reply.addData(QStringLiteral("typeName"), QString::fromUtf8(pedDisk->type->name));
    reply.addData(QStringLiteral("maxPrimaryPartitionCount"), ped_disk_get_max_primary_partition_count(pedDisk));
    reply.addData(QStringLiteral("firstUsableSector"), firstUsableSector);
    reply.addData(QStringLiteral("lastUsableSector"), lastUsableSector);

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

        partitionPath.append(QString::fromLatin1(ped_partition_get_path(pedPartition)));
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
            for (const auto &flag : flagmap)
                if (ped_partition_is_flag_available(pedPartition, flag.pedFlag))
                    // Workaround: libparted claims the hidden flag is available for extended partitions, but
                    // throws an error when we try to set or clear it. So skip this combination. Also see setFlag.
                    if (pedPartition->type != PED_PARTITION_EXTENDED || flag.flag != PartitionTable::FlagHidden)
                        flags |= flag.flag;

        availableFlags.append(static_cast<qint32>(flags));
        // --------------------------------------------------------------------------
        // Get list of active flags

        flags = PartitionTable::FlagNone;
        if (pedPartition->num > 0)
            for (const auto &flag : flagmap)
                if (ped_partition_is_flag_available(pedPartition, flag.pedFlag) && ped_partition_get_flag(pedPartition, flag.pedFlag))
                    flags |= flag.flag;

        activeFlags.append(static_cast<qint32>(flags));
        // --------------------------------------------------------------------------
    }

    reply.addData(QStringLiteral("availableFlags"), availableFlags);
    reply.addData(QStringLiteral("activeFlags"), activeFlags);
    reply.addData(QStringLiteral("partitionPath"), partitionPath);
    reply.addData(QStringLiteral("partitionType"), partitionType);
    reply.addData(QStringLiteral("partitionStart"), partitionStart);
    reply.addData(QStringLiteral("partitionEnd"), partitionEnd);
    reply.addData(QStringLiteral("partitionBusy"), partitionBusy);

    return reply;
}

ActionReply Scan::readsectorsused(const QVariantMap& args)
{
    qint64 rval = -1;

    if (PedDevice* pedDevice = ped_device_get(args[QStringLiteral("deviceNode")].toString().toLocal8Bit().constData()))
        if (PedDisk* pedDisk = ped_disk_new(pedDevice))
            if (PedPartition* pedPartition = ped_disk_get_partition_by_sector(pedDisk, args[QStringLiteral("firstSector")].toLongLong()))
                if (PedFileSystem* pedFileSystem = ped_file_system_open(&pedPartition->geom))
                    if (PedConstraint* pedConstraint = ped_file_system_get_resize_constraint(pedFileSystem))
                        rval = pedConstraint->min_size;

    ActionReply reply;
    reply.addData(QStringLiteral("sectorsUsed"), rval);
    return reply;
}

KAUTH_HELPER_MAIN("org.kde.kpmcore.scan", Scan)
