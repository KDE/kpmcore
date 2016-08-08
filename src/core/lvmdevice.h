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
#include <QStringList>

class PartitionTable;
class CreatePartitionTableOperation;
class SmartStatus;

/** Represents LVM Volume Group.

    Devices are the outermost entity; they contain a PartitionTable that itself contains Partitions.

    @see PartitionTable, Partition
    @author Volker Lanz <vl@fidra.de>
*/
class LIBKPMCORE_EXPORT LvmDevice : public VolumeManagerDevice
{
    Q_DISABLE_COPY(LvmDevice)

public:
    LvmDevice(const QString& name, const QString& iconname = QString());
    ~LvmDevice();

public:
    QList<Partition*> scanPartitions(PartitionTable* pTable) const;
    Partition* scanPartition(const QString& lvPath, PartitionTable* pTable) const;
    QStringList deviceNodeList() const override;
    QStringList lvPathList() const;
    static QStringList s_DirtyPVs;

public:
    static QList<LvmDevice*> scanSystemLVM();

    static qint64 getPeSize(const QString& vgname);
    static qint64 getTotalPE(const QString& vgname);
    static qint64 getAllocatedPE(const QString& vgname);
    static qint64 getFreePE(const QString& vgname);
    static QString getUUID(const QString& vgname);
    static QString getField(const QString& fieldName, const QString& vgname = QString());

    static qint64 getTotalLE(const QString& lvpath);

    static QStringList getPVs(const QString& vgname);
    static QStringList getLVs(const QString& vgname);
    static QStringList getVGs();

    static bool removeLV(Report& report, LvmDevice& dev, Partition& part);
    static bool createLV(Report& report, LvmDevice& dev, Partition& part, const QString& lvname);
    static bool createLVSnapshot(Report& report, LvmDevice& dev, Partition& lvpart, const QString& name, const qint64 extents = 0);
    static bool resizeLV(Report& report, LvmDevice& dev, Partition& part);
    static bool deactivateLV(Report& report, LvmDevice& dev, Partition& part);
    static bool activateLV(Report& report, LvmDevice& dev, Partition& part);

    static bool removePV(Report& report, LvmDevice& dev, const QString& pvPath);
    static bool insertPV(Report& report, LvmDevice& dev, const QString& pvPath);
    static bool movePV(Report& report, LvmDevice& dev, const QString& pvPath, const QStringList& destinations = QStringList());

    static bool removeVG(Report& report, LvmDevice& dev);
    static bool createVG(Report& report, const QString vgname, const QStringList pvlist, const qint32 peSize = 4); // peSize in megabytes
    static bool deactivateVG(Report& report, const LvmDevice& dev);
    static bool activateVG(Report& report, const LvmDevice& dev);

protected:
    void initPartitions();
    qint64 mappedSector(const QString& lvpath, qint64 sector) const override;

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

    QStringList* PVPathList() const {
        return m_PVPathList;
    }

    QMap<QString, qint64>* LVSizeMap() const {
        return m_LVSizeMap;
    }

private:
    qint64 m_peSize;
    qint64 m_totalPE;
    qint64 m_allocPE;
    qint64 m_freePE;
    QString m_UUID;

    mutable QStringList* m_LVPathList;
    mutable QStringList* m_PVPathList;
    mutable QMap<QString, qint64>* m_LVSizeMap;
};

#endif
