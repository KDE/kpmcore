/*************************************************************************
 *  Copyright (C) 2008 by Volker Lanz <vl@fidra.de>                      *
 *  Copyright (C) 2016-2018 by Andrius Štikonas <andrius@stikonas.eu>    *
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

#include "core/diskdevice.h"
#include "core/device_p.h"

#include "core/partitiontable.h"
#include "core/smartstatus.h"

#include <KLocalizedString>

#include <QFile>
#include <QByteArray>

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#ifdef __gnu_linux__
  #include <linux/fs.h>
#endif

#if !defined(BLKPBSZGET)
#define BLKPBSZGET _IO(0x12,123)/* get block physical sector size */
#endif

#define d_ptr std::static_pointer_cast<DiskDevicePrivate>(d)

class DiskDevicePrivate : public DevicePrivate
{
public:
    qint32 m_Heads;
    qint32 m_SectorsPerTrack;
    qint32 m_Cylinders;
    qint64 m_LogicalSectorSize;
    qint64 m_PhysicalSectorSize;
};

static qint64 getPhysicalSectorSize(const QString& device_node)
{
    /*
     * possible ways of getting the physical sector size for a drive:
     * - ioctl(BLKPBSZGET) -- supported with Linux 2.6.32 and later
     * - /sys/block/sda/queue/physical_block_size
     * - libblkid from util-linux 2.17 or later (not implemented)
     */

#if defined(BLKPBSZGET)
    int phSectorSize = -1;
    int fd = open(device_node.toLocal8Bit().constData(), O_RDONLY);
    if (fd != -1) {
        if (ioctl(fd, BLKPBSZGET, &phSectorSize) >= 0) {
            close(fd);
            return phSectorSize;
        }

        close(fd);
    }
#endif

    QFile f(QStringLiteral("/sys/block/%1/queue/physical_block_size").arg(QString(device_node).remove(QStringLiteral("/dev/"))));

    if (f.open(QIODevice::ReadOnly)) {
        QByteArray a = f.readLine();
        return a.trimmed().toInt();
    }

    return -1;
}

/** Constructs a Disk Device with an empty PartitionTable.
    @param name the Device's name, usually some string defined by the manufacturer
    @param deviceNode the Device's node, for example "/dev/sda"
    @param heads the number of heads in CHS notation
    @param numSectors the number of sectors in CHS notation
    @param cylinders the number of cylinders in CHS notation
    @param sectorSize the size of a sector in bytes
*/
DiskDevice::DiskDevice(const QString& name,
                       const QString& deviceNode,
                       qint32 heads,
                       qint32 numSectors,
                       qint32 cylinders,
                       qint64 sectorSize,
                       const QString& iconName)
    : Device(std::make_shared<DiskDevicePrivate>(), name, deviceNode, sectorSize, (static_cast<qint64>(heads) * cylinders * numSectors), iconName, Device::Type::Disk_Device)
{
    d_ptr->m_Heads = heads;
    d_ptr->m_SectorsPerTrack = numSectors;
    d_ptr->m_Cylinders = cylinders;
    d_ptr->m_LogicalSectorSize = sectorSize;
    d_ptr->m_PhysicalSectorSize = getPhysicalSectorSize(deviceNode);
}

qint32 DiskDevice::heads() const
{
    return d_ptr->m_Heads;
}

qint32 DiskDevice::cylinders() const
{
    return d_ptr->m_Cylinders;
}

qint32 DiskDevice::sectorsPerTrack() const
{
    return d_ptr->m_SectorsPerTrack;
}

qint64 DiskDevice::physicalSectorSize() const
{
    return d_ptr->m_PhysicalSectorSize;
}

qint64 DiskDevice::logicalSectorSize() const
{
    return d_ptr->m_LogicalSectorSize;
}

qint64 DiskDevice::totalSectors() const
{
    return static_cast<qint64>(d_ptr->m_Heads) * d_ptr->m_Cylinders * d_ptr->m_SectorsPerTrack;
}

qint64 DiskDevice::cylinderSize() const
{
    return static_cast<qint64>(d_ptr->m_Heads) * d_ptr->m_SectorsPerTrack;
}
