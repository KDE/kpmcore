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

#include "core/lvmdevice.h"
#include "fs/filesystem.h"
#include "fs/filesystemfactory.h"
#include "core/partition.h"

#include "core/partitiontable.h"
#include "util/externalcommand.h"

#include <QRegularExpression>
#include <QStringList>

/** Constructs a representation of LVM device with functionning LV as Partition
    @param name Volume Group name
*/
LvmDevice::LvmDevice(const QString& name, const QString& iconname)
    : VolumeManagerDevice(name,
                          (QStringLiteral("/dev/") + name),
                          getPeSize(name),
                          getTotalPE(name),
                          iconname,
                          Device::LVM_Device)
    , m_peSize(getPeSize(name))
    , m_totalPE(getTotalPE(name))
    , m_allocPE(getAllocatedPE(name))
    , m_freePE(getFreePE(name))
    , m_UUID(getUUID(name))
{
    initPartitions();
}

/*
void LvmDevice::update() const
{
}
*/

void LvmDevice::initPartitions()
{
    qint64 firstUsable = 0;
    qint64 lastusable  = totalPE() - 1;
    PartitionTable* pTable = new PartitionTable(PartitionTable::vmd, firstUsable, lastusable);

    foreach (Partition* p, scanPartitions(*this, pTable)) {
        pTable->append(p);
    }

    setPartitionTable(pTable);
}

/**
   return sorted Partition(LV) Array
*/
QList<Partition*> LvmDevice::scanPartitions(const Device& dev, PartitionTable* pTable) const
{
    QList<Partition*> pList;
    QList<QString> lvNodeList;

    ExternalCommand cmd(QStringLiteral("lvm"),
            { QStringLiteral("lvdisplay"),
              QStringLiteral("--units"),
              QStringLiteral("B"),
              dev.name()});
    if (cmd.run(-1) && cmd.exitCode() == 0) {
        QRegularExpression pathRE(QStringLiteral("LV Path\\h+((\\w|/)+)"));
        QRegularExpressionMatchIterator pathMatch = pathRE.globalMatch(cmd.output());
        while (pathMatch.hasNext()) {
            QRegularExpressionMatch path = pathMatch.next();
            lvNodeList << path.captured(1);
        }
    }

    foreach (QString lvNode, lvNodeList) {
        pList.append(scanPartition(lvNode, dev, pTable));
    }

    return pList;
}

/**
   return sorted Partition(LV) Array
*/
Partition* LvmDevice::scanPartition(const QString& lvPath, const Device& dev, PartitionTable* pTable) const
{
    qint64 startSector;
    qint64 endSector;
    QString mountPoint = QString();
    bool mounted = false;

    ExternalCommand cmd(QStringLiteral("lvm"),
            { QStringLiteral("lvdisplay"),
              QStringLiteral("--units"),
              QStringLiteral("B"),
              QStringLiteral("--maps"),
              lvPath});

    if (cmd.run(-1) && cmd.exitCode() == 0) {

        //TODO: regex for first and last sector of the LV
        //TODO: stringing PE into one large contingiuous array of PE ??
        QRegularExpression re(QStringLiteral("Physical extents\\h+(\\d+)\\sto\\s(\\d+)"));
        QRegularExpressionMatch match = re.match(cmd.output());
        if (match.hasMatch()) {
             startSector = match.captured(1).toLongLong();
             endSector   = match.captured(2).toLongLong();
        }
    }
    FileSystem* fs = FileSystemFactory::create(FileSystem::detectFileSystem(lvPath), startSector, endSector);

    Partition* part = new Partition(pTable,
                    dev,
                    PartitionRole(PartitionRole::Lvm_Lv),
                    fs,
                    startSector,
                    endSector,
                    lvPath,
                    PartitionTable::Flag::FlagLvm,
                    mountPoint,
                    mounted);
    return part;
}

QList<QString> LvmDevice::deviceNodeList() const
{
    QList<QString> devPathList;
    QString cmdOutput = getField(QStringLiteral("pv_name"), name());

    if (cmdOutput.size()) {
        QList<QString> tempPathList = cmdOutput.split(QStringLiteral("\n"), QString::SkipEmptyParts);
        foreach(QString devPath, tempPathList) {
            devPathList.append(devPath.trimmed());
        }
    }

    return devPathList;
}

qint32 LvmDevice::getPeSize(const QString& vgname)
{
    QString val = getField(QStringLiteral("vg_extent_size"), vgname);
    return val.isEmpty() ? -1 : val.toInt();
}

qint32 LvmDevice::getTotalPE(const QString& vgname)
{
    QString val = getField(QStringLiteral("vg_extent_count"), vgname);
    return val.isEmpty() ? -1 : val.toInt();
}

qint32 LvmDevice::getAllocatedPE(const QString& vgname)
{
    return getTotalPE(vgname) - getFreePE(vgname);
}

qint32 LvmDevice::getFreePE(const QString& vgname)
{
    QString val =  getField(QStringLiteral("vg_free_count"), vgname);
    return val.isEmpty() ? -1 : val.toInt();
}

QString LvmDevice::getUUID(const QString& vgname)
{
    QString val = getField(QStringLiteral("vg_uuid"), vgname);
    return val.isEmpty() ? QStringLiteral("---") : val;

}

QString LvmDevice::getField(const QString& fieldName, const QString& vgname)
{
    ExternalCommand cmd(QStringLiteral("lvm"),
            { QStringLiteral("vgs"),
              QStringLiteral("--foreign"),
              QStringLiteral("--readonly"),
              QStringLiteral("--noheadings"),
              QStringLiteral("--units"),
              QStringLiteral("B"),
              QStringLiteral("--nosuffix"),
              QStringLiteral("--options"),
              fieldName,
              vgname });
    if (cmd.run(-1) && cmd.exitCode() == 0) {
        return cmd.output().trimmed();
    }
    return QString();
}
