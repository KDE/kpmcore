/*************************************************************************
 *  Copyright (C) 2016 by Chantara Tith <tith.chantara@gmail.com>        *
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

#if !defined(VOLUMEMANAGERDEVICE__H)

#define VOLUMEMANAGERDEVICE__H

#include "util/libpartitionmanagerexport.h"
#include "core/device.h"

#include <QString>
#include <QStringList>
#include <QObject>
#include <QtGlobal>

class PartitionTable;
class CreatePartitionTableOperation;
class CoreBackend;
class SmartStatus;
class Partition;

/** A Volume Manager of real physical devices represented as an abstract device.

    VolumeManagerDevice is an abstract class of volume manager. e.g: LVM, SoftRAID.
    a device like /dev/sda, /dev/sdb1.

    Devices are the outermost entity; they contain a PartitionTable that itself contains Partitions.

    @see PartitionTable, Partition
*/
class LIBKPMCORE_EXPORT VolumeManagerDevice : public Device
{
    Q_DISABLE_COPY(VolumeManagerDevice)

public:
    /**
     *
     */
    VolumeManagerDevice(const QString& name, const QString& devicenode, const qint32 logicalSize, const qint64 totalLogical, const QString& iconname = QString(), Device::Type type = Device::Unknown_Device);

    /**
     *
     */
    virtual const QStringList deviceNodes() const = 0; /** Return list of physical device or partitions that makes up volumeManagerDevice. */
    virtual const QStringList partitionNodes() const = 0; /** Return list of partitions on the device. */
    virtual qint64 partitionSize(QString& partitionPath) const = 0; /** Return size of provided partition in bytes. */

protected:
    /**
     *
     */
    virtual void initPartitions() = 0;
    /**
     *
     */
    virtual qint64  mappedSector(const QString& devNode, qint64 sector) const = 0;

public:

    /** string deviceNodes together into comma-sperated list
     *
     * */
    virtual QString prettyDeviceNodeList() const;

    /**
     *
     */
    void setTotalLogical(qint64 num) {
        Q_ASSERT(num > 0);
        m_TotalLogical = num;
    }
};

#endif

