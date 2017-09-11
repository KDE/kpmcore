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

#if !defined(KPMCORE_DISKDEVICE_H)

#define KPMCORE_DISKDEVICE_H

#include "util/libpartitionmanagerexport.h"
#include "core/device.h"

#include <QString>
#include <QObject>
#include <QtGlobal>

class PartitionTable;
class CreatePartitionTableOperation;
class CoreBackend;
class SmartStatus;

/** A disk device.

    Represents a device like /dev/sda.

    Devices are the outermost entity; they contain a PartitionTable that itself contains Partitions.

    @see PartitionTable, Partition
    @author Volker Lanz <vl@fidra.de>
*/
class LIBKPMCORE_EXPORT DiskDevice : public Device
{
    Q_DISABLE_COPY(DiskDevice)

    friend class CreatePartitionTableOperation;
    friend class CoreBackend;

public:
    DiskDevice(const QString& name, const QString& deviceNode, qint32 heads, qint32 numSectors, qint32 cylinders, qint64 sectorSize, const QString& iconName = QString());

public:
    qint32 heads() const {
        return m_Heads;    /**< @return the number of heads on the Device in CHS notation */
    }
    qint32 cylinders() const {
        return m_Cylinders;    /**< @return the number of cylinders on the Device in CHS notation */
    }
    qint32 sectorsPerTrack() const {
        return m_SectorsPerTrack;    /**< @return the number of sectors on the Device in CHS notation */
    }
    qint64 physicalSectorSize() const {
        return m_PhysicalSectorSize;    /**< @return the physical sector size the Device uses or -1 if unknown */
    }
    qint64 logicalSectorSize() const {
        return m_LogicalSectorSize;    /**< @return the logical sector size the Device uses */
    }
    qint64 totalSectors() const {
        return static_cast<qint64>(heads()) * cylinders() * sectorsPerTrack();    /**< @return the total number of sectors on the device */
    }
    qint64 cylinderSize() const {
        return static_cast<qint64>(heads()) * sectorsPerTrack();    /**< @return the size of a cylinder on this Device in sectors */
    }

private:
    qint32 m_Heads;
    qint32 m_SectorsPerTrack;
    qint32 m_Cylinders;
    qint64 m_LogicalSectorSize;
    qint64 m_PhysicalSectorSize;
};

#endif
