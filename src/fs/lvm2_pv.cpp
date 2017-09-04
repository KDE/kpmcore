/*************************************************************************
 *  Copyright (C) 2012 by Volker Lanz <vl@fidra.de>                      *
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

#include "fs/lvm2_pv.h"
#include "core/device.h"

#include "util/externalcommand.h"
#include "util/capacity.h"

#include <QString>

#include <KLocalizedString>

namespace FS
{
FileSystem::CommandSupportType lvm2_pv::m_GetUsed = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType lvm2_pv::m_GetLabel = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType lvm2_pv::m_Create = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType lvm2_pv::m_Grow = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType lvm2_pv::m_Shrink = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType lvm2_pv::m_Move = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType lvm2_pv::m_Check = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType lvm2_pv::m_Copy = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType lvm2_pv::m_Backup = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType lvm2_pv::m_SetLabel = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType lvm2_pv::m_UpdateUUID = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType lvm2_pv::m_GetUUID = FileSystem::cmdSupportNone;

lvm2_pv::lvm2_pv(qint64 firstsector, qint64 lastsector,
                 qint64 sectorsused, const QString& label)
    : FileSystem(firstsector, lastsector, sectorsused, label, FileSystem::Lvm2_PV)
    , m_PESize(0)
    , m_TotalPE(0)
    , m_AllocatedPE(0)
{
}

void lvm2_pv::init()
{
    CommandSupportType lvmFound = findExternal(QStringLiteral("lvm")) ? cmdSupportFileSystem : cmdSupportNone;

    m_Create     = lvmFound;
    m_Check      = lvmFound;
    m_Grow       = lvmFound;
    m_Shrink     = lvmFound;
    m_UpdateUUID = lvmFound;
    m_GetUsed    = lvmFound;

    m_Move = (m_Check != cmdSupportNone) ? cmdSupportCore : cmdSupportNone;

    m_GetLabel = cmdSupportCore;
    m_Backup   = cmdSupportCore;
    m_GetUUID  = cmdSupportCore;

    m_GetLabel = cmdSupportNone;
    m_Copy     = cmdSupportNone; // Copying PV can confuse LVM
}

void lvm2_pv::scan(const QString& deviceNode)
{
    getPESize(deviceNode);
    m_AllocatedPE = getAllocatedPE(deviceNode);
    m_TotalPE = getTotalPE(deviceNode);
}

bool lvm2_pv::supportToolFound() const
{
    return
        m_GetUsed != cmdSupportNone &&
        m_Create != cmdSupportNone &&
        m_Check != cmdSupportNone &&
        m_UpdateUUID != cmdSupportNone &&
        m_Grow != cmdSupportNone &&
        m_Shrink != cmdSupportNone &&
        m_Move != cmdSupportNone &&
        m_Backup != cmdSupportNone &&
        m_GetUUID != cmdSupportNone;
}

FileSystem::SupportTool lvm2_pv::supportToolName() const
{
    return SupportTool(QStringLiteral("lvm2"), QUrl(QStringLiteral("http://sourceware.org/lvm2/")));
}

qint64 lvm2_pv::maxCapacity() const
{
    return Capacity::unitFactor(Capacity::Byte, Capacity::EiB);
}

qint64 lvm2_pv::readUsedCapacity(const QString& deviceNode) const
{
    QString pvUsed = getpvField(QStringLiteral("pv_used"), deviceNode);
    QString metadataOffset = getpvField(QStringLiteral("pe_start"), deviceNode);
    return pvUsed.isEmpty() ? -1 : pvUsed.toLongLong() + metadataOffset.toLongLong();
}

bool lvm2_pv::check(Report& report, const QString& deviceNode) const
{
    ExternalCommand cmd(report, QStringLiteral("lvm"), { QStringLiteral("pvck"), QStringLiteral("--verbose"), deviceNode });
    return cmd.run(-1) && cmd.exitCode() == 0;
}

bool lvm2_pv::create(Report& report, const QString& deviceNode)
{
    ExternalCommand cmd(report, QStringLiteral("lvm"), { QStringLiteral("pvcreate"), QStringLiteral("--force"), deviceNode });
    return cmd.run(-1) && cmd.exitCode() == 0;
}

bool lvm2_pv::remove(Report& report, const QString& deviceNode) const
{
    ExternalCommand cmd(report, QStringLiteral("lvm"), { QStringLiteral("pvremove"), QStringLiteral("--force"), QStringLiteral("--force"), QStringLiteral("--yes"), deviceNode });
    return cmd.run(-1) && cmd.exitCode() == 0;
}

bool lvm2_pv::resize(Report& report, const QString& deviceNode, qint64 length) const
{
    bool rval = true;

    qint64 metadataOffset = getpvField(QStringLiteral("pe_start"), deviceNode).toLongLong();

    qint64 lastPE = getTotalPE(deviceNode) - 1; // starts from 0
    if (lastPE > 0) { // make sure that the PV is already in a VG
        qint64 targetPE = (length - metadataOffset) / peSize() - 1; // starts from 0
        if (targetPE < lastPE) { //shrinking FS
            qint64 firstMovedPE = qMax(targetPE + 1, getAllocatedPE(deviceNode)); // starts from 1
            ExternalCommand moveCmd(report,
                                    QStringLiteral("lvm"), {
                                    QStringLiteral("pvmove"),
                                    QStringLiteral("--alloc"),
                                    QStringLiteral("anywhere"),
                                    deviceNode + QStringLiteral(":") + QString::number(firstMovedPE) + QStringLiteral("-") + QString::number(lastPE),
                                    deviceNode + QStringLiteral(":") + QStringLiteral("0-") + QString::number(firstMovedPE - 1)
                                    });
            rval = moveCmd.run(-1) && (moveCmd.exitCode() == 0 || moveCmd.exitCode() == 5); // FIXME: exit code 5: NO data to move
        }
    }

    ExternalCommand cmd(report, QStringLiteral("lvm"), {
                                QStringLiteral("pvresize"),
                                QStringLiteral("--yes"),
                                QStringLiteral("--setphysicalvolumesize"),
                                QString::number(length) + QStringLiteral("B"),
                                deviceNode });
    return rval && cmd.run(-1) && cmd.exitCode() == 0;
}

bool lvm2_pv::resizeOnline(Report& report, const QString& deviceNode, const QString& mountPoint, qint64 length) const
{
    Q_UNUSED(mountPoint)
    return resize(report, deviceNode, length);
}

bool lvm2_pv::updateUUID(Report& report, const QString& deviceNode) const
{
    ExternalCommand cmd(report, QStringLiteral("lvm"), { QStringLiteral("pvchange"), QStringLiteral("--uuid"), deviceNode });
    return cmd.run(-1) && cmd.exitCode() == 0;
}

QString lvm2_pv::readUUID(const QString& deviceNode) const
{
    return getpvField(QStringLiteral("pv_uuid"), deviceNode);
}

bool lvm2_pv::mount(Report& report, const QString& deviceNode, const QString& mountPoint)
{
    Q_UNUSED(report)
    Q_UNUSED(deviceNode)
    Q_UNUSED(mountPoint)
    return false;
}

bool lvm2_pv::unmount(Report& report, const QString& deviceNode)
{
    Q_UNUSED(deviceNode)
    Q_UNUSED(report)
    return false;
}

bool lvm2_pv::canMount(const QString& deviceNode, const QString& mountPoint) const
{
    Q_UNUSED(deviceNode)
    Q_UNUSED(mountPoint)
    return false;
}

bool lvm2_pv::canUnmount(const QString& deviceNode) const
{
    Q_UNUSED(deviceNode)
    return false;
}

qint64 lvm2_pv::getTotalPE(const QString& deviceNode)
{
    QString pvPeCount = getpvField(QStringLiteral("pv_pe_count"), deviceNode);
    return pvPeCount.isEmpty() ? -1 : pvPeCount.toLongLong();
}

qint64 lvm2_pv::getAllocatedPE(const QString& deviceNode)
{
    QString pvPeAllocCount = getpvField(QStringLiteral("pv_pe_alloc_count"), deviceNode);
    return pvPeAllocCount.isEmpty() ? -1 : pvPeAllocCount.toLongLong();
}

void lvm2_pv::getPESize(const QString& deviceNode)
{
    QString vgExtentSize = getpvField(QStringLiteral("vg_extent_size"), deviceNode);
    m_PESize = vgExtentSize.isEmpty() ? -1 : vgExtentSize.toLongLong();
}

/** Get pvs command output with field name
 *
 *  @param fieldName LVM field name
 *  @param deviceNode path to PV
 *  @return raw output of pvs command, usually with many spaces
 */
QString  lvm2_pv::getpvField(const QString& fieldName, const QString& deviceNode)
{
    QStringList args = { QStringLiteral("pvs"),
                    QStringLiteral("--foreign"),
                    QStringLiteral("--readonly"),
                    QStringLiteral("--noheadings"),
                    QStringLiteral("--units"),
                    QStringLiteral("B"),
                    QStringLiteral("--nosuffix"),
                    QStringLiteral("--options"),
                    fieldName };
    if (!deviceNode.isEmpty()) {
        args << deviceNode;
    }
    ExternalCommand cmd(QStringLiteral("lvm"), args, QProcess::ProcessChannelMode::SeparateChannels);
    if (cmd.run(-1) && cmd.exitCode() == 0) {
        return cmd.output().trimmed();
    }
    return QString();
}

QString lvm2_pv::getVGName(const QString& deviceNode)
{
    return getpvField(QStringLiteral("vg_name"), deviceNode);
}

QList<LvmPV> lvm2_pv::getPVinNode(const PartitionNode* parent)
{
    QList<LvmPV> partitions;
    if (parent == nullptr)
        return partitions;

    for (const auto &node : parent->children()) {
        const Partition* p = dynamic_cast<const Partition*>(node);

        if (p == nullptr)
            continue;

        if (node->children().size() > 0)
            partitions.append(getPVinNode(node));

        // FIXME: reenable newly created PVs (before applying) once everything works
        if(p->fileSystem().type() == FileSystem::Lvm2_PV && p->deviceNode() == p->partitionPath())
            partitions.append(LvmPV(p->mountPoint(), p));

        if(p->fileSystem().type() == FileSystem::Luks && p->deviceNode() == p->partitionPath())
            partitions.append(LvmPV(p->mountPoint(), p, true));
    }

    return partitions;
}

/** construct a list of Partition objects for LVM PVs that are either unused or belong to some VG.
 *
 *  @param devices list of Devices which we scan for LVM PVs
 *  @return list of LVM PVs
 */
QList<LvmPV> lvm2_pv::getPVs(const QList<Device*>& devices)
{
    QList<LvmPV> partitions;
    for (auto const &d : devices)
        partitions.append(getPVinNode(d->partitionTable()));

    return partitions;
}

}

QList<LvmPV> LVM::pvList;

LvmPV::LvmPV(const QString vgName, const Partition* p, bool isLuks)
    : m_vgName(vgName)
    , m_p(p)
    , m_isLuks(isLuks)
{
}
