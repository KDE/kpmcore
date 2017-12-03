/*************************************************************************
 *  Copyright (C) 2010, 2011, 2012 by Volker Lanz <vl@fidra.de>          *
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

#include "plugins/libparted/libpartedpartitiontable.h"
#include "plugins/libparted/libpartedbackend.h"

#include "backend/corebackend.h"
#include "backend/corebackendmanager.h"

#include "core/partition.h"
#include "core/device.h"

#include "fs/filesystem.h"

#include "util/report.h"
#include "util/externalcommand.h"

#include <KLocalizedString>

#include <unistd.h>

LibPartedPartitionTable::LibPartedPartitionTable(PedDevice* device) :
    CoreBackendPartitionTable(),
    m_PedDevice(device),
    m_PedDisk(nullptr)
{
}

LibPartedPartitionTable::~LibPartedPartitionTable()
{
    ped_disk_destroy(m_PedDisk);
}

bool LibPartedPartitionTable::open()
{
    m_PedDisk = ped_disk_new(pedDevice());

    return m_PedDisk != nullptr;
}

bool LibPartedPartitionTable::commit(quint32 timeout)
{
    return commit(pedDisk(), timeout);
}

bool LibPartedPartitionTable::commit(PedDisk* pd, quint32 timeout)
{
    if (pd == nullptr)
        return false;

    bool rval = ped_disk_commit_to_dev(pd);

    if (rval)
        rval = ped_disk_commit_to_os(pd);

    if (!ExternalCommand(QStringLiteral("udevadm"), QStringList() << QStringLiteral("settle") << QStringLiteral("--timeout=") + QString::number(timeout)).run() &&
            !ExternalCommand(QStringLiteral("udevsettle"), QStringList() << QStringLiteral("--timeout=") + QString::number(timeout)).run())
        sleep(timeout);

    return rval;
}

static const struct {
    FileSystem::Type type;
    QString name;
} mapFileSystemTypeToLibPartedName[] = {
    { FileSystem::Btrfs, QStringLiteral("btrfs") },
    { FileSystem::Ext2, QStringLiteral("ext2") },
    { FileSystem::Ext3, QStringLiteral("ext3") },
    { FileSystem::Ext4, QStringLiteral("ext4") },
    { FileSystem::LinuxSwap, QStringLiteral("linux-swap") },
    { FileSystem::Fat16, QStringLiteral("fat16") },
    { FileSystem::Fat32, QStringLiteral("fat32") },
    { FileSystem::Nilfs2, QStringLiteral("nilfs2") },
    { FileSystem::Ntfs, QStringLiteral("ntfs") },
    { FileSystem::Exfat, QStringLiteral("ntfs") },
    { FileSystem::ReiserFS, QStringLiteral("reiserfs") },
    { FileSystem::Reiser4, QStringLiteral("reiserfs") },
    { FileSystem::Xfs, QStringLiteral("xfs") },
    { FileSystem::Jfs, QStringLiteral("jfs") },
    { FileSystem::Hfs, QStringLiteral("hfs") },
    { FileSystem::HfsPlus, QStringLiteral("hfs+") },
    { FileSystem::Ufs, QStringLiteral("ufs") },
    { FileSystem::Udf, QStringLiteral("ntfs") },
    { FileSystem::Iso9660, QStringLiteral("iso9660") }
};

static PedFileSystemType* getPedFileSystemType(FileSystem::Type t)
{
    for (quint32 i = 0; i < sizeof(mapFileSystemTypeToLibPartedName) / sizeof(mapFileSystemTypeToLibPartedName[0]); i++)
        if (mapFileSystemTypeToLibPartedName[i].type == t)
            return ped_file_system_type_get(mapFileSystemTypeToLibPartedName[i].name.toLocal8Bit().constData());

    // if we didn't find anything, go with ext2 as a safe fallback
    return ped_file_system_type_get("ext2");
}

QString LibPartedPartitionTable::createPartition(Report& report, const Partition& partition)
{
    Q_ASSERT(partition.devicePath() == QString::fromLocal8Bit(pedDevice()->path));

    QString rval = QString();

    // According to libParted docs, PedPartitionType can be "nullptr if unknown". That's obviously wrong,
    // it's a typedef for an enum. So let's use something the libparted devs will hopefully never
    // use...
    PedPartitionType pedType = static_cast<PedPartitionType>(0xffffffff);

    if (partition.roles().has(PartitionRole::Extended))
        pedType = PED_PARTITION_EXTENDED;
    else if (partition.roles().has(PartitionRole::Logical))
        pedType = PED_PARTITION_LOGICAL;
    else if (partition.roles().has(PartitionRole::Primary))
        pedType = PED_PARTITION_NORMAL;

    if (pedType == static_cast<int>(0xffffffff)) {
        report.line() << xi18nc("@info:progress", "Unknown partition role for new partition <filename>%1</filename> (roles: %2)", partition.deviceNode(), partition.roles().toString());
        return QString();
    }

    PedFileSystemType* pedFsType = (partition.roles().has(PartitionRole::Extended) || partition.fileSystem().type() == FileSystem::Unformatted) ? nullptr : getPedFileSystemType(partition.fileSystem().type());

    PedPartition* pedPartition = ped_partition_new(pedDisk(), pedType, pedFsType, partition.firstSector(), partition.lastSector());

    if (pedPartition == nullptr) {
        report.line() << xi18nc("@info:progress", "Failed to create new partition <filename>%1</filename>.", partition.deviceNode());
        return QString();
    }

    PedConstraint* pedConstraint = nullptr;
    PedGeometry* pedGeometry = ped_geometry_new(pedDevice(), partition.firstSector(), partition.length());

    if (pedGeometry)
        pedConstraint = ped_constraint_exact(pedGeometry);
    ped_geometry_destroy(pedGeometry);

    if (pedConstraint == nullptr) {
        report.line() << i18nc("@info:progress", "Failed to create a new partition: could not get geometry for constraint.");
        return QString();
    }

    if (ped_disk_add_partition(pedDisk(), pedPartition, pedConstraint)) {
        char *pedPath = ped_partition_get_path(pedPartition);
        rval = QString::fromLocal8Bit(pedPath);
        free(pedPath);
    }
    else {
        report.line() << xi18nc("@info:progress", "Failed to add partition <filename>%1</filename> to device <filename>%2</filename>.", partition.deviceNode(), QString::fromLocal8Bit(pedDisk()->dev->path));
        report.line() << LibPartedBackend::lastPartedExceptionMessage();
    }

    ped_constraint_destroy(pedConstraint);

    return rval;
}

bool LibPartedPartitionTable::deletePartition(Report& report, const Partition& partition)
{
    Q_ASSERT(partition.devicePath() == QString::fromLocal8Bit(pedDevice()->path));

    bool rval = false;

    PedPartition* pedPartition = partition.roles().has(PartitionRole::Extended)
                                 ? ped_disk_extended_partition(pedDisk())
                                 : ped_disk_get_partition_by_sector(pedDisk(), partition.firstSector());

    if (pedPartition) {
        rval = ped_disk_delete_partition(pedDisk(), pedPartition);

        if (!rval)
            report.line() << xi18nc("@info:progress", "Could not delete partition <filename>%1</filename>.", partition.deviceNode());
    } else
        report.line() << xi18nc("@info:progress", "Deleting partition failed: Partition to delete (<filename>%1</filename>) not found on disk.", partition.deviceNode());

    return rval;
}

bool LibPartedPartitionTable::updateGeometry(Report& report, const Partition& partition, qint64 sector_start, qint64 sector_end)
{
    Q_ASSERT(partition.devicePath() == QString::fromLocal8Bit(pedDevice()->path));

    bool rval = false;

    PedPartition* pedPartition = (partition.roles().has(PartitionRole::Extended))
                                 ? ped_disk_extended_partition(pedDisk())
                                 : ped_disk_get_partition_by_sector(pedDisk(), partition.firstSector());

    if (pedPartition) {
        if (PedGeometry* pedGeometry = ped_geometry_new(pedDevice(), sector_start, sector_end - sector_start + 1)) {
            if (PedConstraint* pedConstraint = ped_constraint_exact(pedGeometry)) {
                if (ped_disk_set_partition_geom(pedDisk(), pedPartition, pedConstraint, sector_start, sector_end))
                    rval = true;
                else
                    report.line() << xi18nc("@info:progress", "Could not set geometry for partition <filename>%1</filename> while trying to resize/move it.", partition.deviceNode());
                ped_constraint_destroy(pedConstraint);
            } else
                report.line() << xi18nc("@info:progress", "Could not get constraint for partition <filename>%1</filename> while trying to resize/move it.", partition.deviceNode());
            ped_geometry_destroy(pedGeometry);
        } else
            report.line() << xi18nc("@info:progress", "Could not get geometry for partition <filename>%1</filename> while trying to resize/move it.", partition.deviceNode());
    } else
        report.line() << xi18nc("@info:progress", "Could not open partition <filename>%1</filename> while trying to resize/move it.", partition.deviceNode());

    return rval;
}

bool LibPartedPartitionTable::clobberFileSystem(Report& report, const Partition& partition)
{
    bool rval = false;

    if (PedPartition* pedPartition = ped_disk_get_partition_by_sector(pedDisk(), partition.firstSector())) {
        if (pedPartition->type == PED_PARTITION_NORMAL || pedPartition->type == PED_PARTITION_LOGICAL) {
            if (ped_device_open(pedDevice())) {
                //reiser4 stores "ReIsEr4" at sector 128 with a sector size of 512 bytes

                // We need to use memset instead of = {0} because clang sucks.
                const long long zeroes_length = pedDevice()->sector_size*129;
                char* zeroes = new char[zeroes_length];
                memset(zeroes, 0, zeroes_length*sizeof(char));

                rval = ped_geometry_write(&pedPartition->geom, zeroes, 0, 129);
                delete[] zeroes;

                if (!rval)
                    report.line() << xi18nc("@info:progress", "Failed to erase filesystem signature on partition <filename>%1</filename>.", partition.deviceNode());

                ped_device_close(pedDevice());
            }
        } else
            rval = true;
    } else
        report.line() << xi18nc("@info:progress", "Could not delete file system on partition <filename>%1</filename>: Failed to get partition.", partition.deviceNode());

    return rval;
}

#if defined LIBPARTED_FS_RESIZE_LIBRARY_SUPPORT
static void pedTimerHandler(PedTimer* pedTimer, void*)
{
    CoreBackendManager::self()->backend()->emitProgress(pedTimer->frac * 100);
}
#endif

bool LibPartedPartitionTable::resizeFileSystem(Report& report, const Partition& partition, qint64 newLength)
{
    bool rval = false;

#if defined LIBPARTED_FS_RESIZE_LIBRARY_SUPPORT
    if (PedGeometry* originalGeometry = ped_geometry_new(pedDevice(), partition.fileSystem().firstSector(), partition.fileSystem().length())) {
        if (PedFileSystem* pedFileSystem = ped_file_system_open(originalGeometry)) {
            if (PedGeometry* resizedGeometry = ped_geometry_new(pedDevice(), partition.fileSystem().firstSector(), newLength)) {
                PedTimer* pedTimer = ped_timer_new(pedTimerHandler, nullptr);
                rval = ped_file_system_resize(pedFileSystem, resizedGeometry, pedTimer);
                ped_timer_destroy(pedTimer);

                if (!rval)
                    report.line() << xi18nc("@info:progress", "Could not resize file system on partition <filename>%1</filename>.", partition.deviceNode());
                ped_geometry_destroy(resizedGeometry);
            } else
                report.line() << xi18nc("@info:progress", "Could not get geometry for resized partition <filename>%1</filename> while trying to resize the file system.", partition.deviceNode());

            ped_file_system_close(pedFileSystem);
        } else
            report.line() << xi18nc("@info:progress", "Could not open partition <filename>%1</filename> while trying to resize the file system.", partition.deviceNode());
        ped_geometry_destroy(originalGeometry);
    } else
        report.line() << xi18nc("@info:progress", "Could not read geometry for partition <filename>%1</filename> while trying to resize the file system.", partition.deviceNode());
#else
    Q_UNUSED(report)
    Q_UNUSED(partition)
    Q_UNUSED(newLength)
#endif

    return rval;
}

FileSystem::Type LibPartedPartitionTable::detectFileSystemBySector(Report& report, const Device& device, qint64 sector)
{
    PedPartition* pedPartition = ped_disk_get_partition_by_sector(pedDisk(), sector);

    char* pedPath = ped_partition_get_path(pedPartition);
    FileSystem::Type type = FileSystem::Unknown;
    if (pedPartition && pedPath)
        type = CoreBackendManager::self()->backend()->detectFileSystem(QString::fromLocal8Bit(pedPath));
    else
        report.line() << xi18nc("@info:progress", "Could not determine file system of partition at sector %1 on device <filename>%2</filename>.", sector, device.deviceNode());
    free(pedPath);

    return type;
}

bool LibPartedPartitionTable::setPartitionSystemType(Report& report, const Partition& partition)
{
    PedFileSystemType* pedFsType = (partition.roles().has(PartitionRole::Extended) || partition.fileSystem().type() == FileSystem::Unformatted) ? nullptr : getPedFileSystemType(partition.fileSystem().type());
    if (pedFsType == nullptr) {
        report.line() << xi18nc("@info:progress", "Could not update the system type for partition <filename>%1</filename>.", partition.deviceNode());
        report.line() << xi18nc("@info:progress", "No file system defined.");
        return false;
    }

    PedPartition* pedPartition = ped_disk_get_partition_by_sector(pedDisk(), partition.firstSector());
    if (pedPartition == nullptr) {
        report.line() << xi18nc("@info:progress", "Could not update the system type for partition <filename>%1</filename>.", partition.deviceNode());
        report.line() << xi18nc("@info:progress", "No partition found at sector %1.", partition.firstSector());
        return false;
    }

    return ped_partition_set_system(pedPartition, pedFsType) != 0;
}

bool LibPartedPartitionTable::setFlag(Report& report, const Partition& partition, PartitionTable::Flag flag, bool state)
{
    PedPartition* pedPartition;
    if (partition.roles().has(PartitionRole::Extended))
        pedPartition = ped_disk_extended_partition(pedDisk());
    else
        pedPartition = ped_disk_get_partition_by_sector(pedDisk(), partition.firstSector());
    if (pedPartition == nullptr) {
        QString deviceNode = QString::fromUtf8(pedDevice()->path);
        report.line() << xi18nc("@info:progress", "Could not find partition <filename>%1</filename> on device <filename>%2</filename> to set partition flags.", partition.deviceNode(), deviceNode);
        return false;
    }

    const PedPartitionFlag f = LibPartedBackend::getPedFlag(flag);

    // ignore flags that don't exist for this partition
    if (!ped_partition_is_flag_available(pedPartition, f)) {
        report.line() << xi18nc("@info:progress", "The flag \"%1\" is not available on the partition's partition table.", PartitionTable::flagName(flag));
        return true;
    }

    // Workaround: libparted claims the hidden flag is available for extended partitions, but
    // throws an error when we try to set or clear it. So skip this combination.
    if (pedPartition->type == PED_PARTITION_EXTENDED && flag == PartitionTable::FlagHidden)
        return true;

    if (!ped_partition_set_flag(pedPartition, f, state ? 1 : 0))
        return false;

    return true;
}
