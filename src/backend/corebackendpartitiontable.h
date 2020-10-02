/*
    SPDX-FileCopyrightText: 2010-2011 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2014-2018 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2015 Teo Mrnjavac <teo@kde.org>
    SPDX-FileCopyrightText: 2015 Chris Campbell <c.j.campbell@ed.ac.uk>
    SPDX-FileCopyrightText: 2020 Gaël PORTAY <gael.portay@collabora.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KPMCORE_COREBACKENDPARTITIONTABLE_H
#define KPMCORE_COREBACKENDPARTITIONTABLE_H

#include "core/partitiontable.h"
#include "fs/filesystem.h"

#include <QtGlobal>

class CoreBackendPartition;
class Report;
class Partition;

/**
  * Interface class to represent a partition table in the backend.
  * @author Volker Lanz <vl@fidra.de>
  */
class CoreBackendPartitionTable
{
public:
    virtual ~CoreBackendPartitionTable() {}

public:
    /**
      * Open the partition table
      * @return true on success
      */
    virtual bool open() = 0;

    /**
      * Commit changes to the partition table to disk and to the OS.
      * @param timeout timeout in seconds to wait for the commit to succeed
      * @return true on success
    */
    virtual bool commit(quint32 timeout = 10) = 0;

    /**
      * Delete a partition.
      * @param report the report to write information to
      * @param partition the Partition to delete
      * @return true on success
      */
    virtual bool deletePartition(Report& report, const Partition& partition) = 0;

    /**
      * Delete a file system on disk so it cannot be detected anymore.
      * @param report the report to write information to
      * @param partition the Partition for which to clobber the file system
      * @return true on success
      */
    virtual bool clobberFileSystem(Report& report, const Partition& partition) = 0;

    /**
      * Resize a file system to a new length.
      * @param report the report to write information to
      * @param partition the partition the FileSystem to resize is on
      * @param newLength the new length for the FileSystem in sectors
      * @return true on success
      */
    virtual bool resizeFileSystem(Report& report, const Partition& partition, qint64 newLength) = 0;

    /**
      * Detect which FileSystem is present at a given start sector.
      * @param report the report to write information to
      * @param device the Device on which the FileSystem resides
      * @param sector the sector where to look for a FileSystem
      * @return the detected FileSystem::Type
      */
    virtual FileSystem::Type detectFileSystemBySector(Report& report, const Device& device, qint64 sector) = 0;

    /**
      * Create a new partition.
      * @param report the report to write information to
      * @param partition the new partition to create on disk
      * @return the new number the OS sees the partition under (e.g. 7 for "/dev/sda7") or -1 on failure
      */
    virtual QString createPartition(Report& report, const Partition& partition) = 0;

    /**
      * Update the geometry for a partition in the partition table.
      * @param report the report to write information to
      * @param partition the partition to update the geometry for
      * @param sector_start the new start sector for the partition
      * @param sector_end the new last sector for the partition
      * @return true on success
      */
    virtual bool updateGeometry(Report& report, const Partition& partition, qint64 sector_start, qint64 sector_end) = 0;

    /**
     * Get the UUID of a partition in the partition table (GPT only).
     * The partition UUID is known as PARTUUID by several utilities. The device-manager links
     * the device under /dev/disk/by-partuuid/<uuid>.
     * @param report the report to write information to
     * @param partition the partition to get the UUID for
     * @return the partition UUID
     */
    virtual QString getPartitionUUID(Report& report, const Partition& partition) = 0;

    /**
     * Set the label of a partition in the partition table (GPT only).
     * The label is set in the GPT partition name entry. The partition name is known as PARTLABEL by
     * several utilities. The device-manager links the device under /dev/disk/by-partlabel/<label>.
     * @param report the report to write information to
     * @param partition the partition to set the label for
     * @param label the new label for the partition
     * @return true on success
     */
    virtual bool setPartitionLabel(Report& report, const Partition& partition, const QString& label) = 0;

    /**
     * Set the UUID of a partition in the partition table (GPT only).
     * The partition UUID is known as PARTUUID by several utilities. The device-manager links
     * the device under /dev/disk/by-partuuid/<uuid>.
     * @param report the report to write information to
     * @param partition the partition to set the UUID for
     * @param uuid the new UUID for the partition
     * @return true on success
     */
    virtual bool setPartitionUUID(Report& report, const Partition& partition, const QString& uuid) = 0;

    /**
     * Set the attributes of a partition in the partition table (GPT only).
     * @param report the report to write information to
     * @param partition the partition to set the attributes for
     * @param attrs the new attributes for the partition
     * @return true on success
     */
    virtual bool setPartitionAttributes(Report& report, const Partition& partition, quint64 attrs) = 0;

    /**
     * Set the system type (e.g. 83 for Linux) of a partition. The type to set is taken from
     * the partition's file system.
     * @param report the report to write information to
     * @param partition the partition to set the system type for
     * @return true on success
     */
    virtual bool setPartitionSystemType(Report& report, const Partition& partition) = 0;

    /**
      * Set a flag for the partition
      * @param report the Report to write information to
      * @param flag the flag to set
      * @param state the state to set the flag to (i.e., on or off)
      * @return true on success
      */
    virtual bool setFlag(Report& report, const Partition& partition, PartitionTable::Flag flag, bool state) = 0;
};

#endif
