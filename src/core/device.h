/*************************************************************************
 *  Copyright (C) 2008 by Volker Lanz <vl@fidra.de>                      *
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

#ifndef KPMCORE_DEVICE_H
#define KPMCORE_DEVICE_H

#include "util/libpartitionmanagerexport.h"

#include <QString>
#include <QObject>

#include <memory>

class PartitionTable;
class CreatePartitionTableOperation;
class CoreBackend;
class SmartStatus;
class DevicePrivate;

/** A device description.

    Represents a device like /dev/sda. Contains information about
    the device (name, status, size ..) but does not operate on
    the device itself. @see CoreBackendDevice

    Devices are the outermost entity; they contain a PartitionTable that itself contains Partitions.

    @see PartitionTable, Partition
    @author Volker Lanz <vl@fidra.de>
*/
class LIBKPMCORE_EXPORT Device : public QObject
{
    Device &operator=(const Device &) = delete;

    friend class CreatePartitionTableOperation;
    friend class CoreBackend;

public:
    enum class Type {
        Unknown_Device,
        Disk_Device,
        LVM_Device, /* VG */
        SoftwareRAID_Device, /* software RAID device, i.e. mdraid */
        FakeRAID_Device, /* fake RAID device, i.e. dmraid */
    };

    explicit Device(std::shared_ptr<DevicePrivate> d_ptr, const QString& name, const QString& deviceNode, const qint64 logicalSectorSize, const qint64 totalLogicalSectors, const QString& iconName = QString(), Device::Type type = Device::Type::Disk_Device);

public:
    explicit Device(const Device& other);
    virtual ~Device();

    virtual bool operator==(const Device& other) const;
    virtual bool operator!=(const Device& other) const;

    /**< @return the Device's name, usually some manufacturer string */
    virtual QString& name();
    virtual const QString& name() const;

    /**< @return the Device's node, for example "/dev/sda" */
    virtual const QString& deviceNode() const;

    /**< @return the logical sector size the Device uses (would be extent size for e.g. LVM devices) */
    virtual qint64 logicalSize() const;

    /**< @return the total number of logical sectors on the device */
    virtual qint64 totalLogical() const;

    /**< @return the Device's PartitionTable */
    virtual PartitionTable* partitionTable();
    virtual const PartitionTable* partitionTable() const;

    /**
     * Change the description of the partition table for different one.
     * The device itself is not changed; use CreatePartitionTableOperation
     * for that. The Device instance becomes the owner of @p ptable .
     */
    virtual void setPartitionTable(PartitionTable* ptable);

    virtual qint64 capacity() const { /**< @return the Device's capacity in bytes */
        return logicalSize() * totalLogical();
    }

    /**< @return suggested icon name for this Device */
    virtual const QString& iconName() const;

    /**< @param name set the new Icon for this Device */
    virtual void setIconName(const QString& name);

    virtual SmartStatus& smartStatus();
    virtual const SmartStatus& smartStatus() const;

    virtual Device::Type type() const;

    virtual QString prettyName() const;

protected:
    std::shared_ptr<DevicePrivate> d;
};

#endif
