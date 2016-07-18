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
#include "util/report.h"

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
    QList<Partition*> scanPartitions(const LvmDevice& dev, PartitionTable* pTable) const;
    Partition* scanPartition(const QString& lvPath, const LvmDevice& dev, PartitionTable* pTable) const;

    static qint64 getPeSize(const QString& vgname);
    static qint64 getTotalPE(const QString& vgname);
    static qint64 getAllocatedPE(const QString& vgname);
    static qint64 getFreePE(const QString& vgname);
    static QString getUUID(const QString& vgname);
    static QString getField(const QString& fieldName, const QString& vgname = QString());

    static qint64 getTotalLE(const QString& lvpath);

    static QStringList getPVs(const QString& vgname);
    static QStringList getLVs(const QString& vgname);

    static bool removeLV(Report& report, LvmDevice& dev, Partition& part);
    static bool createLV(Report& report, LvmDevice& dev, Partition& part, const QString& lvname);
    static bool resizeLV(Report& report, LvmDevice& dev, Partition& part);

    static bool removePV(Report& report, LvmDevice& dev, const QString& pvPath);
    static bool insertPV(Report& report, LvmDevice& dev, const QString& pvPath);
    static bool movePV(Report& report, LvmDevice& dev, const QString& pvPath, const QStringList& destinations = QStringList());

    static bool removeVG(Report& report, LvmDevice& dev);
    static bool createVG(Report& report, const QString vgname, const QStringList pvlist, const qint32 peSize = 4); // peSize in megabytes

protected:
    void initPartitions();
    QList<QString> deviceNodeList() const override;
    qint64 mappedSector(const QString& lvpath, qint64 sector) const override;

    QList<QString> lvPathList() const;

public:
    qint64 peSize() const {
        return m_peSize;
    }
    qint64 totalPE() const {
        return m_totalPE;
    }
    qint64 allocatedPE() const {
        return m_allocPE;
    }
    qint64 freePE() const {
        return m_freePE;
    }
    QString UUID() const {
        return m_UUID;
    }

private:
    qint64 m_peSize;
    qint64 m_totalPE;
    qint64 m_allocPE;
    qint64 m_freePE;
    QString m_UUID;

};

#endif

