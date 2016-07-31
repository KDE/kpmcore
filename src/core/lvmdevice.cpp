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
#include "fs/lvm2_pv.h"
#include "fs/luks.h"
#include "fs/filesystemfactory.h"
#include "core/partition.h"

#include "core/partitiontable.h"
#include "util/externalcommand.h"
#include "util/helpers.h"

#include <QRegularExpression>
#include <QStringList>

#include <KDiskFreeSpaceInfo>
#include <KLocalizedString>
#include <KMountPoint>

/** Constructs a representation of LVM device with functionning LV as Partition
 *
 *  @param name Volume Group name
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

void LvmDevice::initPartitions()
{
    qint64 firstUsable = 0;
    qint64 lastusable  = totalPE() - 1;
    PartitionTable* pTable = new PartitionTable(PartitionTable::vmd, firstUsable, lastusable);

    foreach (Partition* p, scanPartitions(pTable)) {
        pTable->append(p);
    }

    pTable->updateUnallocated(*this);

    setPartitionTable(pTable);
}

/**
 *  @return sorted Partition(LV) Array
 */
QList<Partition*> LvmDevice::scanPartitions(PartitionTable* pTable) const
{
    QList<Partition*> pList;
    foreach (QString lvPath, lvPathList()) {
        pList.append(scanPartition(lvPath, pTable));
    }
    return pList;
}

/**
 * @return sorted Partition (LV) Array
 */
Partition* LvmDevice::scanPartition(const QString& lvpath, PartitionTable* pTable) const
{
    /*
     * NOTE:
     * LVM partition has 2 different start and end sector values
     * 1. representing the actual LV start from 0 -> size of LV - 1
     * 2. representing abstract LV's sector inside a VG partitionTable
     *    start from last sector + 1 of last Partitions -> size of LV - 1
     * Reason for this is for the LV Partition to work nicely with other parts of the codebase
     * without too many special cases.
     */

    qint64 lvSize = getTotalLE(lvpath);
    qint64 startSector = mappedSector(lvpath, 0);
    qint64 endSector = startSector + lvSize - 1;

    FileSystem::Type type = FileSystem::detectFileSystem(lvpath);
    FileSystem* fs = FileSystemFactory::create(type, 0, lvSize - 1);

    bool mounted = isMounted(lvpath);
    QString mountPoint = QString();

    KMountPoint::List mountPointList = KMountPoint::currentMountPoints(KMountPoint::NeedRealDeviceName);
    mountPointList.append(KMountPoint::possibleMountPoints(KMountPoint::NeedRealDeviceName));

    PartitionRole::Roles r = PartitionRole::Lvm_Lv;

    if (type == FileSystem::Luks) {
        r |= PartitionRole::Luks;
        FS::luks* luksFs = dynamic_cast<FS::luks*>(fs);
        QString mapperNode = FS::luks::mapperName(lvpath);
        bool isCryptOpen = !mapperNode.isEmpty();
        luksFs->setCryptOpen(isCryptOpen);
        luksFs->setLogicalSectorSize(logicalSize());

        if (isCryptOpen) {
            luksFs->loadInnerFileSystem(lvpath, mapperNode);
            mountPoint = mountPointList.findByDevice(mapperNode) ?
                         mountPointList.findByDevice(mapperNode)->mountPoint() :
                         QString();
            mounted = isMounted(mapperNode);
            if (mounted) {
                const KDiskFreeSpaceInfo freeSpaceInfo = KDiskFreeSpaceInfo::freeSpaceInfo(mountPoint);
                if (freeSpaceInfo.isValid() && mountPoint != QString())
                    luksFs->setSectorsUsed((freeSpaceInfo.used() + luksFs->getPayloadOffset(lvpath)) / logicalSize());
            }
        } else {
            mounted = false;
        }
        luksFs->setMounted(mounted);
    } else {
        mountPoint = mountPointList.findByDevice(lvpath) ?
                     mountPointList.findByDevice(lvpath)->mountPoint() :
                     QString();
        const KDiskFreeSpaceInfo freeSpaceInfo = KDiskFreeSpaceInfo::freeSpaceInfo(mountPoint);

        //TODO: test used space report. probably incorrect
        if (logicalSize() > 0) {
            if (mounted && freeSpaceInfo.isValid() && mountPoint != QString()) {
                fs->setSectorsUsed(freeSpaceInfo.used() / logicalSize());
            } else if (fs->supportGetUsed() == FileSystem::cmdSupportFileSystem) {
                fs->setSectorsUsed(fs->readUsedCapacity(lvpath) / logicalSize());
            }
        }
    }

    if (fs->supportGetLabel() != FileSystem::cmdSupportNone) {
        fs->setLabel(fs->readLabel(lvpath));
    }

    Partition* part = new Partition(pTable,
                    *this,
                    PartitionRole(r),
                    fs,
                    startSector,
                    endSector,
                    lvpath,
                    PartitionTable::Flag::FlagNone,
                    mountPoint,
                    mounted);
    return part;
}

QList<LvmDevice*> LvmDevice::scanSystemLVM()
{
    QList<LvmDevice*> lvmList;

    QString output = getField(QStringLiteral("vg_name"));
    if (!output.isEmpty()) {
        QStringList vgnameList = output.split(QStringLiteral("\n"), QString::SkipEmptyParts);
        foreach(QString vgname, vgnameList) {
            lvmList.append(new LvmDevice(vgname.trimmed()));
        }
    }

    return lvmList;
}

qint64 LvmDevice::mappedSector(const QString& lvpath, qint64 sector) const
{
    qint64 mSector = 0;
    QList<QString> lvpathList = lvPathList();
    qint32 devIndex = lvpathList.indexOf(lvpath);

    if (devIndex) {
        for (int i = 0; i < devIndex; i++) {
            //TODO: currently going over the same LV again and again is wasteful. Could use some more optimization
            mSector += getTotalLE(lvpathList[i]);
        }
        mSector += sector;
    }
    return mSector;
}

QStringList LvmDevice::getPVs(const QString& vgname)
{
    QStringList devPathList;
    QString cmdOutput = getField(QStringLiteral("pv_name"), vgname);

    if (cmdOutput.size()) {
        QStringList tempPathList = cmdOutput.split(QStringLiteral("\n"), QString::SkipEmptyParts);
        foreach(QString devPath, tempPathList) {
            devPathList.append(devPath.trimmed());
        }
    }
    return devPathList;
}

QList<QString> LvmDevice::deviceNodeList() const
{
    return getPVs(name());
}

QStringList LvmDevice::getLVs(const QString& vgname)
{
    QStringList lvPathList;
    QString cmdOutput = getField(QStringLiteral("lv_path"), vgname);

    if (cmdOutput.size()) {
        QStringList tempPathList = cmdOutput.split(QStringLiteral("\n"), QString::SkipEmptyParts);
        foreach(QString lvPath, tempPathList) {
            lvPathList.append(lvPath.trimmed());
        }
    }
    return lvPathList;
}

QList<QString> LvmDevice::lvPathList() const
{
    return getLVs(name());
}

qint64 LvmDevice::getPeSize(const QString& vgname)
{
    QString val = getField(QStringLiteral("vg_extent_size"), vgname);
    return val.isEmpty() ? -1 : val.toInt();
}

qint64 LvmDevice::getTotalPE(const QString& vgname)
{
    QString val = getField(QStringLiteral("vg_extent_count"), vgname);
    return val.isEmpty() ? -1 : val.toInt();
}

qint64 LvmDevice::getAllocatedPE(const QString& vgname)
{
    return getTotalPE(vgname) - getFreePE(vgname);
}

qint64 LvmDevice::getFreePE(const QString& vgname)
{
    QString val =  getField(QStringLiteral("vg_free_count"), vgname);
    return val.isEmpty() ? -1 : val.toInt();
}

QString LvmDevice::getUUID(const QString& vgname)
{
    QString val = getField(QStringLiteral("vg_uuid"), vgname);
    return val.isEmpty() ? QStringLiteral("---") : val;

}

/** Get LVM vgs command output with field name
 *
 * @param fieldName LVM field name
 * @param vgname the name of LVM Volume Group
 * @return raw output of command output, usully with many spaces within the returned string
 * */

QString LvmDevice::getField(const QString& fieldName, const QString& vgname)
{
    QStringList args = { QStringLiteral("vgs"),
              QStringLiteral("--foreign"),
              QStringLiteral("--readonly"),
              QStringLiteral("--noheadings"),
              QStringLiteral("--units"),
              QStringLiteral("B"),
              QStringLiteral("--nosuffix"),
              QStringLiteral("--options"),
              fieldName };
    if  (!vgname.isEmpty()) {
        args << vgname;
    }
    ExternalCommand cmd(QStringLiteral("lvm"), args);
    if (cmd.run(-1) && cmd.exitCode() == 0) {
        return cmd.output().trimmed();
    }
    return QString();
}

qint64 LvmDevice::getTotalLE(const QString& lvpath)
{
    ExternalCommand cmd(QStringLiteral("lvm"),
            { QStringLiteral("lvdisplay"),
              lvpath});

    if (cmd.run(-1) && cmd.exitCode() == 0) {
        QRegularExpression re(QStringLiteral("Current LE\\h+(\\d+)"));
        QRegularExpressionMatch match = re.match(cmd.output());
        if (match.hasMatch()) {
             return  match.captured(1).toInt();
        }
    }
    return -1;
}

bool LvmDevice::removeLV(Report& report, LvmDevice& dev, Partition& part)
{
    ExternalCommand cmd(report, QStringLiteral("lvm"),
            { QStringLiteral("lvremove"),
              QStringLiteral("--yes"),
              part.partitionPath()});

    if (cmd.run(-1) && cmd.exitCode() == 0) {
        //TODO: remove Partition from PartitionTable and delete from memory ??
        dev.partitionTable()->remove(&part);
        return  true;
    }
    return false;
}

bool LvmDevice::createLV(Report& report, LvmDevice& dev, Partition& part, const QString& lvname)
{
    ExternalCommand cmd(report, QStringLiteral("lvm"),
            { QStringLiteral("lvcreate"),
              QStringLiteral("--yes"),
              QStringLiteral("--extents"),
              QString::number(part.length()),
              QStringLiteral("--name"),
              lvname,
              dev.name()});

    return (cmd.run(-1) && cmd.exitCode() == 0);
}

bool LvmDevice::resizeLV(Report& report, LvmDevice& dev, Partition& part)
{
    Q_UNUSED(dev);
    //TODO: thorough tests and add warning that it could currupt the user data.
    ExternalCommand cmd(report, QStringLiteral("lvm"),
            { QStringLiteral("lvresize"),
              QStringLiteral("--force"), // this command could corrupt user data
              QStringLiteral("--yes"),
              QStringLiteral("--extents"),
              QString::number(part.length()),
              part.partitionPath()});

    return (cmd.run(-1) && cmd.exitCode() == 0);
}

bool LvmDevice::removePV(Report& report, LvmDevice& dev, const QString& pvPath)
{
    ExternalCommand cmd(report, QStringLiteral("lvm"),
            { QStringLiteral("vgreduce"),
              //QStringLiteral("--yes"), // potentially corrupt user data
              dev.name(),
              pvPath});

    return (cmd.run(-1) && cmd.exitCode() == 0);
}

bool LvmDevice::insertPV(Report& report, LvmDevice& dev, const QString& pvPath)
{
    ExternalCommand cmd(report, QStringLiteral("lvm"),
            { QStringLiteral("vgextend"),
              //QStringLiteral("--yes"), // potentially corrupt user data
              dev.name(),
              pvPath});

    return (cmd.run(-1) && cmd.exitCode() == 0);
}
bool LvmDevice::movePV(Report& report, LvmDevice& dev, const QString& pvPath, const QStringList& destinations)
{
    Q_UNUSED(dev);

    if (FS::lvm2_pv::getAllocatedPE(pvPath) <= 0) {
        return true;
    }

    QStringList args = QStringList();
    args << QStringLiteral("pvmove");
    args << pvPath;
    if (!destinations.isEmpty()) {
        foreach (QString destPath, destinations) {
            args << destPath.trimmed();
        }
    }

    ExternalCommand cmd(report, QStringLiteral("lvm"), args);
    return (cmd.run(-1) && cmd.exitCode() == 0);
}

bool LvmDevice::createVG(Report& report, const QString vgname, const QStringList pvlist, const qint32 peSize)
{
    QStringList args = QStringList();
    args << QStringLiteral("vgcreate") << QStringLiteral("--physicalextentsize") << QString::number(peSize);
    args << vgname;
    foreach (QString pvnode, pvlist) {
        args << pvnode.trimmed();
    }
    ExternalCommand cmd(report, QStringLiteral("lvm"), args);

    return (cmd.run(-1) && cmd.exitCode() == 0);
}

bool LvmDevice::removeVG(Report& report, LvmDevice& dev)
{
    bool deactivated = false;
    ExternalCommand deactivate(report, QStringLiteral("lvm"),
            { QStringLiteral("vgchange"),
              QStringLiteral("--activate"), QStringLiteral("n"),
              dev.name() });
    deactivated = deactivate.run(-1) && deactivate.exitCode() == 0;

    ExternalCommand cmd(report, QStringLiteral("lvm"),
            { QStringLiteral("vgremove"),
              dev.name() });
    return (deactivated && cmd.run(-1) && cmd.exitCode() == 0);
}
