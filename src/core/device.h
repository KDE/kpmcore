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

#if !defined(KPMCORE_DEVICE_H)

#define KPMCORE_DEVICE_H

#include "util/libpartitionmanagerexport.h"

#include <QString>
#include <QObject>

class PartitionTable;
class CreatePartitionTableOperation;
class CoreBackend;
class SmartStatus;

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
    enum Type {
        Disk_Device = 0,
        LVM_Device  = 1, /* VG */
        RAID_Device = 2, /* software RAID device */
        Unknown_Device = 4
    };

protected:
    explicit Device(const QString& name, const QString& deviceNode, const qint64 logicalSize, const qint64 totalLogical, const QString& iconName = QString(), Device::Type type = Device::Disk_Device);

public:
    explicit Device(const Device& other);
    virtual ~Device();

public:
    virtual bool operator==(const Device& other) const;
    virtual bool operator!=(const Device& other) const;

    virtual QString& name() {
        return m_Name;    /**< @return the Device's name, usually some manufacturer string */
    }

    virtual const QString& name() const {
        return m_Name;    /**< @return the Device's name, usually some manufacturer string */
    }

    virtual const QString& deviceNode() const {
        return m_DeviceNode;    /**< @return the Device's node, for example "/dev/sda" */
    }

    virtual PartitionTable* partitionTable() {
        return m_PartitionTable;    /**< @return the Device's PartitionTable */
    }

    virtual const PartitionTable* partitionTable() const {
        return m_PartitionTable;    /**< @return the Device's PartitionTable */
    }

    virtual qint64 capacity() const { /**< @return the Device's capacity in bytes */
        return logicalSize() * totalLogical();
    }

    virtual void setIconName(const QString& name) {
        m_IconName = name;
    }

    virtual const QString& iconName() const {
        return m_IconName;    /**< @return suggested icon name for this Device */
    }

    virtual SmartStatus& smartStatus() {
        return *m_SmartStatus;
    }

    virtual const SmartStatus& smartStatus() const {
        return *m_SmartStatus;
    }

    /**
     * Change the description of the partition table for different one.
     * The device itself is not changed; use CreatePartitionTableOperation
     * for that. The Device instance becomes the owner of @p ptable .
     */
    virtual void setPartitionTable(PartitionTable* ptable) {
        m_PartitionTable = ptable;
    }

    virtual qint64 logicalSize() const {
        return m_LogicalSize;
    }

    virtual qint64 totalLogical() const {
        return m_TotalLogical;
    }

    virtual Device::Type type() const {
        return m_Type;
    }

    virtual QString prettyName() const;

protected:
    QString m_Name;
    QString m_DeviceNode;
    qint64  m_LogicalSize;
    qint64  m_TotalLogical;
    PartitionTable* m_PartitionTable;
    QString m_IconName;
    SmartStatus* m_SmartStatus;
    Device::Type m_Type;
};

#endif
