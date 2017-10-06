/*************************************************************************
 *  Copyright (C) 2016 by Chantara Tith <tith.chantara@gmail.com>        *
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

#if !defined(KPMCORE_LVMDEVICE_H)

#define KPMCORE_LVMDEVICE_H

#include "core/device.h"
#include "core/volumemanagerdevice.h"
#include "util/libpartitionmanagerexport.h"

#include <QHash>
#include <QString>
#include <QObject>
#include <QStringList>
#include <QtGlobal>
#include <QVector>

class PartitionTable;
class Report;
class Partition;
class SmartStatus;

/** Representation of LVM Volume Group(VG).

    Devices are the outermost entity; they contain a PartitionTable that itself contains Partitions.

    @see Device, VolumeManagerDevice, PartitionTable, Partition
*/
class LIBKPMCORE_EXPORT LvmDevice : public VolumeManagerDevice
{
    Q_DISABLE_COPY(LvmDevice)

public:
    LvmDevice(const QString& name, const QString& iconName = QString());
    ~LvmDevice();

public:
    const QStringList deviceNodes() const override;
    const QStringList partitionNodes() const override;
    qint64 partitionSize(QString& partitionPath) const override;

    static QVector<const Partition*> s_DirtyPVs;

public:
    static void scanSystemLVM(QList<Device*>& devices);

    static const QStringList getVGs();
    static const QStringList getLVs(const QString& vgName);

    static qint64 getPeSize(const QString& vgName);
    static qint64 getTotalPE(const QString& vgName);
    static qint64 getAllocatedPE(const QString& vgName);
    static qint64 getFreePE(const QString& vgName);
    static QString getUUID(const QString& vgName);
    static QString getField(const QString& fieldName, const QString& vgName = QString());

    static qint64 getTotalLE(const QString& lvPath);

    static bool removeLV(Report& report, LvmDevice& d, Partition& p);
    static bool createLV(Report& report, LvmDevice& d, Partition& p, const QString& lvName);
    static bool createLVSnapshot(Report& report, Partition& p, const QString& name, const qint64 extents = 0);
    static bool resizeLV(Report& report, Partition& p);
    static bool deactivateLV(Report& report, const Partition& p);
    static bool activateLV(const QString& deviceNode);

    static bool removePV(Report& report, LvmDevice& d, const QString& pvPath);
    static bool insertPV(Report& report, LvmDevice& d, const QString& pvPath);
    static bool movePV(Report& report, const QString& pvPath, const QStringList& destinations = QStringList());

    static bool removeVG(Report& report, LvmDevice& d);
    static bool createVG(Report& report, const QString vgName, const QVector<const Partition*>& pvList, const qint32 peSize = 4); // peSize in megabytes
    static bool deactivateVG(Report& report, const LvmDevice& d);
    static bool activateVG(Report& report, const LvmDevice& d);

protected:

    void initPartitions() override;
    const QList<Partition*> scanPartitions(PartitionTable* pTable) const;
    Partition* scanPartition(const QString& lvPath, PartitionTable* pTable) const;
    qint64 mappedSector(const QString& lvPath, qint64 sector) const override;

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
    QStringList* LVPathList() const {
        return m_LVPathList;
    }
    QVector <const Partition*>& physicalVolumes() {
        return m_PVs;
    }
    const QVector <const Partition*>& physicalVolumes() const {
        return m_PVs;
    }

protected:
    QHash<QString, qint64>* LVSizeMap() const {
        return m_LVSizeMap;
    }

private:
    qint64 m_peSize;
    qint64 m_totalPE;
    qint64 m_allocPE;
    qint64 m_freePE;
    QString m_UUID;

    mutable QStringList* m_LVPathList;
    QVector <const Partition*> m_PVs;
    mutable QHash<QString, qint64>* m_LVSizeMap;

};

#endif
