/*************************************************************************
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
#include <QObject>
#include <QtGlobal>

class PartitionTable;
class CreatePartitionTableOperation;
class CoreBackend;
class SmartStatus;
class Partition;

/** A device.

    Represents a device like /dev/sda.

    Devices are the outermost entity; they contain a PartitionTable that itself contains Partitions.

    @see PartitionTable, Partition
    @author Volker Lanz <vl@fidra.de>
*/
class LIBKPMCORE_EXPORT VolumeManagerDevice : public Device
{
    Q_DISABLE_COPY(VolumeManagerDevice)

protected:
    VolumeManagerDevice(const QString& name, const QString& devicenode, const qint32 logicalSize, const qint64 totalLogical, const QString& iconname = QString(), Device::Type type = Device::Unknown_Device);
    virtual QList<QString> deviceNodeList() const = 0; /** Return list of physical device or partitions that makes up volumeManagerDevice */

public:
    //virtual void refresh() const = 0; /* VG infos can be changed, unlike disk_device  */
    //virtual Qlist<Partition*> listDevices() const = 0;
    virtual QString prettyDeviceNodeList() const;

private:
};

#endif

