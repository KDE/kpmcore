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
#include "core/partition.h"
#include "fs/filesystem.h"
#include "fs/lvm2_pv.h"
#include "fs/luks.h"
#include "fs/filesystemfactory.h"

#include "core/partitiontable.h"
#include "util/externalcommand.h"
#include "util/helpers.h"
#include "util/report.h"

#include <QRegularExpression>

#include <KDiskFreeSpaceInfo>
#include <KLocalizedString>
#include <KMountPoint>

/** Constructs a representation of LVM device with functionning LV as Partitions
 *
 *  @param vgName Volume Group name
 *  @param iconName Icon representing LVM Volume group
 */
LvmDevice::LvmDevice(const QString& vgName, const QString& iconName)
    : VolumeManagerDevice(vgName,
                          (QStringLiteral("/dev/") + vgName),
                          getPeSize(vgName),
                          getTotalPE(vgName),
                          iconName,
                          Device::LVM_Device)
{
    m_peSize  = logicalSize();
    m_totalPE = totalLogical();
    m_freePE  = getFreePE(vgName);
    m_allocPE = m_totalPE - m_freePE;
    m_UUID    = getUUID(vgName);
    m_PVPathList = new QStringList(getPVs(vgName));
    m_LVPathList = new QStringList(getLVs(vgName));
    m_LVSizeMap  = new QMap<QString, qint64>();

    initPartitions();
}

QStringList LvmDevice::s_DirtyPVs;

LvmDevice::~LvmDevice()
{
    delete m_PVPathList;
    delete m_LVPathList;
    delete m_LVSizeMap;
}

void LvmDevice::initPartitions()
{
    qint64 firstUsable = 0;
    qint64 lastusable  = totalPE() - 1;
    PartitionTable* pTable = new PartitionTable(PartitionTable::vmd, firstUsable, lastusable);

    for (const auto &p : scanPartitions(pTable)) {
        LVSizeMap()->insert(p->partitionPath(), p->length());
        pTable->append(p);
    }

    pTable->updateUnallocated(*this);

    setPartitionTable(pTable);
}

/**
 *  @return sorted Partition(LV) Array
 */
const QList<Partition*> LvmDevice::scanPartitions(PartitionTable* pTable) const
{
    QList<Partition*> pList;
    for (const auto &lvPath : partitionNodes()) {
        pList.append(scanPartition(lvPath, pTable));
    }
    return pList;
}

/**
 * @param lvPath LVM Logical Volume path
 * @param pTable Abstract partition table representing partitions of LVM Volume Group
 * @return sorted Partition (LV) Array
 */
Partition* LvmDevice::scanPartition(const QString& lvPath, PartitionTable* pTable) const
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

    activateLV(lvPath);

    qint64 lvSize = getTotalLE(lvPath);
    qint64 startSector = mappedSector(lvPath, 0);
    qint64 endSector = startSector + lvSize - 1;

    FileSystem::Type type = FileSystem::detectFileSystem(lvPath);
    FileSystem* fs = FileSystemFactory::create(type, 0, lvSize - 1);

    bool mounted = isMounted(lvPath);
    QString mountPoint = QString();

    KMountPoint::List mountPointList = KMountPoint::currentMountPoints(KMountPoint::NeedRealDeviceName);
    mountPointList.append(KMountPoint::possibleMountPoints(KMountPoint::NeedRealDeviceName));

    PartitionRole::Roles r = PartitionRole::Lvm_Lv;

    if (type == FileSystem::Luks) {
        r |= PartitionRole::Luks;
        FS::luks* luksFs = static_cast<FS::luks*>(fs);
        QString mapperNode = FS::luks::mapperName(lvPath);
        bool isCryptOpen = !mapperNode.isEmpty();
        luksFs->setCryptOpen(isCryptOpen);
        luksFs->setLogicalSectorSize(logicalSize());

        if (isCryptOpen) {
            luksFs->loadInnerFileSystem(lvPath, mapperNode);
            mountPoint = mountPointList.findByDevice(mapperNode) ?
                         mountPointList.findByDevice(mapperNode)->mountPoint() :
                         QString();
            mounted = isMounted(mapperNode);
            if (mounted) {
                const KDiskFreeSpaceInfo freeSpaceInfo = KDiskFreeSpaceInfo::freeSpaceInfo(mountPoint);
                if (freeSpaceInfo.isValid() && mountPoint != QString())
                    luksFs->setSectorsUsed((freeSpaceInfo.used() + luksFs->getPayloadOffset(lvPath)) / logicalSize());
            }
        } else {
            mounted = false;
        }
        luksFs->setMounted(mounted);
    } else {
        mountPoint = mountPointList.findByDevice(lvPath) ?
                     mountPointList.findByDevice(lvPath)->mountPoint() :
                     QString();
        const KDiskFreeSpaceInfo freeSpaceInfo = KDiskFreeSpaceInfo::freeSpaceInfo(mountPoint);

        //TODO: test used space report. probably incorrect
        if (logicalSize() > 0) {
            if (mounted && freeSpaceInfo.isValid() && mountPoint != QString()) {
                fs->setSectorsUsed(freeSpaceInfo.used() / logicalSize());
            } else if (fs->supportGetUsed() == FileSystem::cmdSupportFileSystem) {
                fs->setSectorsUsed(fs->readUsedCapacity(lvPath) / logicalSize());
            }
        }
    }

    if (fs->supportGetLabel() != FileSystem::cmdSupportNone) {
        fs->setLabel(fs->readLabel(lvPath));
    }
    if (fs->supportGetUUID() != FileSystem::cmdSupportNone)
        fs->setUUID(fs->readUUID(lvPath));

    Partition* part = new Partition(pTable,
                    *this,
                    PartitionRole(r),
                    fs,
                    startSector,
                    endSector,
                    lvPath,
                    PartitionTable::Flag::FlagNone,
                    mountPoint,
                    mounted);
    return part;
}

/**
 *
 */
QList<LvmDevice*> LvmDevice::scanSystemLVM()
{
    QList<LvmDevice*> lvmList;
    for (const auto &vgName : getVGs()) {
        lvmList.append(new LvmDevice(vgName));
    }
    return lvmList;
}

qint64 LvmDevice::mappedSector(const QString& lvPath, qint64 sector) const
{
    qint64 mSector = 0;
    QList<QString> lvpathList = partitionNodes();
    qint32 devIndex = lvpathList.indexOf(lvPath);

    if (devIndex) {
        for (int i = 0; i < devIndex; i++) {
            mSector += LVSizeMap()->value(lvpathList[i]);
        }
        mSector += sector;
    }
    return mSector;
}

const QStringList LvmDevice::deviceNodes() const
{
    return *PVPathList();
}

const QStringList LvmDevice::partitionNodes() const
{
    return *LVPathList();
}

qint64 LvmDevice::partitionSize(QString& partitionPath) const
{
    return LVSizeMap()->value(partitionPath);
}

const QStringList LvmDevice::getVGs()
{
    QStringList vgList;
    QString output = getField(QStringLiteral("vg_name"));
    if (!output.isEmpty()) {
        const QStringList vgNameList = output.split(QStringLiteral("\n"), QString::SkipEmptyParts);
        for (const auto &vgName : vgNameList) {
            vgList.append(vgName.trimmed());
        }
    }
    return vgList;
}

const QStringList LvmDevice::getPVs(const QString& vgName)
{
    QStringList devPathList;
    QString cmdOutput = getField(QStringLiteral("pv_name"), vgName);

    if (cmdOutput.size()) {
        const QStringList tempPathList = cmdOutput.split(QStringLiteral("\n"), QString::SkipEmptyParts);
        for (const auto &devPath : tempPathList) {
            devPathList.append(devPath.trimmed());
        }
    }
    return devPathList;
}

const QStringList LvmDevice::getLVs(const QString& vgName)
{
    QStringList lvPathList;
    QString cmdOutput = getField(QStringLiteral("lv_path"), vgName);

    if (cmdOutput.size()) {
        const QStringList tempPathList = cmdOutput.split(QStringLiteral("\n"), QString::SkipEmptyParts);
        for (const auto &lvPath : tempPathList) {
            lvPathList.append(lvPath.trimmed());
        }
    }
    return lvPathList;
}

qint64 LvmDevice::getPeSize(const QString& vgName)
{
    QString val = getField(QStringLiteral("vg_extent_size"), vgName);
    return val.isEmpty() ? -1 : val.toInt();
}

qint64 LvmDevice::getTotalPE(const QString& vgName)
{
    QString val = getField(QStringLiteral("vg_extent_count"), vgName);
    return val.isEmpty() ? -1 : val.toInt();
}

qint64 LvmDevice::getAllocatedPE(const QString& vgName)
{
    return getTotalPE(vgName) - getFreePE(vgName);
}

qint64 LvmDevice::getFreePE(const QString& vgName)
{
    QString val =  getField(QStringLiteral("vg_free_count"), vgName);
    return val.isEmpty() ? -1 : val.toInt();
}

QString LvmDevice::getUUID(const QString& vgName)
{
    QString val = getField(QStringLiteral("vg_uuid"), vgName);
    return val.isEmpty() ? QStringLiteral("---") : val;

}

/** Get LVM vgs command output with field name
 *
 * @param fieldName LVM field name
 * @param vgName the name of LVM Volume Group
 * @return raw output of command output, usully with many spaces within the returned string
 * */

QString LvmDevice::getField(const QString& fieldName, const QString& vgName)
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
    if  (!vgName.isEmpty()) {
        args << vgName;
    }
    ExternalCommand cmd(QStringLiteral("lvm"), args);
    if (cmd.run(-1) && cmd.exitCode() == 0) {
        return cmd.output().trimmed();
    }
    return QString();
}

qint64 LvmDevice::getTotalLE(const QString& lvPath)
{
    ExternalCommand cmd(QStringLiteral("lvm"),
            { QStringLiteral("lvdisplay"),
              lvPath});

    if (cmd.run(-1) && cmd.exitCode() == 0) {
        QRegularExpression re(QStringLiteral("Current LE\\h+(\\d+)"));
        QRegularExpressionMatch match = re.match(cmd.output());
        if (match.hasMatch()) {
             return  match.captured(1).toInt();
        }
    }
    return -1;
}

bool LvmDevice::removeLV(Report& report, LvmDevice& d, Partition& p)
{
    ExternalCommand cmd(report, QStringLiteral("lvm"),
            { QStringLiteral("lvremove"),
              QStringLiteral("--yes"),
              p.partitionPath()});

    if (cmd.run(-1) && cmd.exitCode() == 0) {
        //TODO: remove Partition from PartitionTable and delete from memory ??
        d.partitionTable()->remove(&p);
        return  true;
    }
    return false;
}

bool LvmDevice::createLV(Report& report, LvmDevice& d, Partition& p, const QString& lvName)
{
    ExternalCommand cmd(report, QStringLiteral("lvm"),
            { QStringLiteral("lvcreate"),
              QStringLiteral("--yes"),
              QStringLiteral("--extents"),
              QString::number(p.length()),
              QStringLiteral("--name"),
              lvName,
              d.name()});

    return (cmd.run(-1) && cmd.exitCode() == 0);
}

bool LvmDevice::createLVSnapshot(Report& report, Partition& p, const QString& name, const qint64 extents)
{
    QString numExtents = (extents > 0) ? QString::number(extents) :
        QString::number(p.length());
    ExternalCommand cmd(report, QStringLiteral("lvm"),
            { QStringLiteral("lvcreate"),
              QStringLiteral("--yes"),
              QStringLiteral("--extents"),
              numExtents,
              QStringLiteral("--snapshot"),
              QStringLiteral("--name"),
              name,
              p.partitionPath() });
    return (cmd.run(-1) && cmd.exitCode() == 0);
}

bool LvmDevice::resizeLV(Report& report, Partition& p)
{
    //TODO: thorough tests and add warning that it could currupt the user data.
    ExternalCommand cmd(report, QStringLiteral("lvm"),
            { QStringLiteral("lvresize"),
              QStringLiteral("--force"), // this command could corrupt user data
              QStringLiteral("--yes"),
              QStringLiteral("--extents"),
              QString::number(p.length()),
              p.partitionPath()});

    return (cmd.run(-1) && cmd.exitCode() == 0);
}

bool LvmDevice::removePV(Report& report, LvmDevice& d, const QString& pvPath)
{
    ExternalCommand cmd(report, QStringLiteral("lvm"),
            { QStringLiteral("vgreduce"),
              //QStringLiteral("--yes"), // potentially corrupt user data
              d.name(),
              pvPath});

    return (cmd.run(-1) && cmd.exitCode() == 0);
}

bool LvmDevice::insertPV(Report& report, LvmDevice& d, const QString& pvPath)
{
    ExternalCommand cmd(report, QStringLiteral("lvm"),
            { QStringLiteral("vgextend"),
              //QStringLiteral("--yes"), // potentially corrupt user data
              d.name(),
              pvPath});

    return (cmd.run(-1) && cmd.exitCode() == 0);
}
bool LvmDevice::movePV(Report& report, const QString& pvPath, const QStringList& destinations)
{

    if (FS::lvm2_pv::getAllocatedPE(pvPath) <= 0) {
        return true;
    }

    QStringList args = QStringList();
    args << QStringLiteral("pvmove");
    args << pvPath;
    if (!destinations.isEmpty()) {
        for (const auto &destPath : destinations) {
            args << destPath.trimmed();
        }
    }

    ExternalCommand cmd(report, QStringLiteral("lvm"), args);
    return (cmd.run(-1) && cmd.exitCode() == 0);
}

bool LvmDevice::createVG(Report& report, const QString vgName, const QStringList pvList, const qint32 peSize)
{
    QStringList args = QStringList();
    args << QStringLiteral("vgcreate") << QStringLiteral("--physicalextentsize") << QString::number(peSize);
    args << vgName;
    for (const auto &pvNode : pvList) {
        args << pvNode.trimmed();
    }
    ExternalCommand cmd(report, QStringLiteral("lvm"), args);

    return (cmd.run(-1) && cmd.exitCode() == 0);
}

bool LvmDevice::removeVG(Report& report, LvmDevice& d)
{
    bool deactivated = deactivateVG(report, d);
    ExternalCommand cmd(report, QStringLiteral("lvm"),
            { QStringLiteral("vgremove"),
              d.name() });
    return (deactivated && cmd.run(-1) && cmd.exitCode() == 0);
}

bool LvmDevice::deactivateVG(Report& report, const LvmDevice& d)
{
    ExternalCommand deactivate(report, QStringLiteral("lvm"),
            { QStringLiteral("vgchange"),
              QStringLiteral("--activate"), QStringLiteral("n"),
              d.name() });
    return deactivate.run(-1) && deactivate.exitCode() == 0;
}

bool LvmDevice::deactivateLV(Report& report, const Partition& p)
{
    ExternalCommand deactivate(report, QStringLiteral("lvm"),
            { QStringLiteral("lvchange"),
              QStringLiteral("--activate"), QStringLiteral("n"),
              p.partitionPath() });
    return deactivate.run(-1) && deactivate.exitCode() == 0;
}

bool LvmDevice::activateVG(Report& report, const LvmDevice& d)
{
    ExternalCommand deactivate(report, QStringLiteral("lvm"),
            { QStringLiteral("vgchange"),
              QStringLiteral("--activate"), QStringLiteral("y"),
              d.name() });
    return deactivate.run(-1) && deactivate.exitCode() == 0;
}

bool LvmDevice::activateLV(const QString& lvPath)
{
    ExternalCommand deactivate(QStringLiteral("lvm"),
            { QStringLiteral("lvchange"),
              QStringLiteral("--activate"), QStringLiteral("y"),
              lvPath });
    return deactivate.run(-1) && deactivate.exitCode() == 0;
}
