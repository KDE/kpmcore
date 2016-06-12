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

#if !defined(LVMDEVICE__H)

#define LVMDEVICE__H

#include "core/volumemanagerdevice.h"

#include "util/libpartitionmanagerexport.h"

#include <QString>
#include <QObject>
#include <QtGlobal>

class PartitionTable;
class CreatePartitionTableOperation;
class SmartStatus;

/** A device.

    Represents a device like /dev/sda.

    Devices are the outermost entity; they contain a PartitionTable that itself contains Partitions.

    @see PartitionTable, Partition
    @author Volker Lanz <vl@fidra.de>
*/
class LIBKPMCORE_EXPORT LvmDevice : public VolumeManagerDevice
{
    Q_DISABLE_COPY(LvmDevice)

public:
    LvmDevice(const QString& name, const QString& iconname = QString());

public:
    QList<Partition*> scanPartitions(const Device& dev, PartitionTable* pTable) const;
    Partition* scanPartition(const QString& lvPath, const Device& dev, PartitionTable* pTable) const;

    static qint32 getPeSize(const QString& name);
    static qint32 getTotalPE(const QString& name);
    static qint32 getAllocatedPE(const QString& name);
    static qint32 getFreePE(const QString& name);
    static QString getUUID(const QString& name);
    static QString getField(const QString& fieldName, const QString& vgname = QString());

protected:
    void initPartitions();
    QList<QString> deviceNodeList() const override;

public:
    qint32 peSize() const {
        return m_peSize;
    }
    qint32 totalPE() const {
        return m_totalPE;
    }
    qint32 allocatedPE() const {
        return m_allocPE;
    }
    qint32 freePE() const {
        return m_freePE;
    }
    QString UUID() const {
        return m_UUID;
    }

private:
    qint32 m_peSize;
    qint32 m_totalPE;
    qint32 m_allocPE;
    qint32 m_freePE;
    QString m_UUID;
};

#endif

