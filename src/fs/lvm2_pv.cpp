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

#include "util/externalcommand.h"
#include "util/capacity.h"

#include <QString>
#include <QRegularExpression>

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

lvm2_pv::lvm2_pv(qint64 firstsector, qint64 lastsector, qint64 sectorsused, const QString& label) :
    FileSystem(firstsector, lastsector, sectorsused, label, FileSystem::Lvm2_PV)
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

bool lvm2_pv::supportToolFound() const
{
    return
        m_GetUsed != cmdSupportNone &&
//          m_GetLabel != cmdSupportNone &&
//          m_SetLabel != cmdSupportNone &&
        m_Create != cmdSupportNone &&
        m_Check != cmdSupportNone &&
        m_UpdateUUID != cmdSupportNone &&
        m_Grow != cmdSupportNone &&
        m_Shrink != cmdSupportNone &&
//          m_Copy != cmdSupportNone &&
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
    QString val = getpvField(QStringLiteral("pv_used"), deviceNode);
    return val.isEmpty() ? -1 : val.toLongLong();
}

bool lvm2_pv::check(Report& report, const QString& deviceNode) const
{
    ExternalCommand cmd(report, QStringLiteral("lvm"), { QStringLiteral("pvck"), QStringLiteral("--verbose"), deviceNode });
    return cmd.run(-1) && cmd.exitCode() == 0;
}

bool lvm2_pv::create(Report& report, const QString& deviceNode) const
{
    ExternalCommand cmd(report, QStringLiteral("lvm"), { QStringLiteral("pvcreate"), deviceNode });
    return cmd.run(-1) && cmd.exitCode() == 0;
}

bool lvm2_pv::remove(Report& report, const QString& deviceNode) const
{
//      TODO: check if PV is a member of an exported VG
    ExternalCommand cmd(report, QStringLiteral("lvm"), { QStringLiteral("pvremove"), QStringLiteral("--force"), QStringLiteral("--force"), QStringLiteral("--yes"), deviceNode });
    return cmd.run(-1) && cmd.exitCode() == 0;
}

bool lvm2_pv::resize(Report& report, const QString& deviceNode, qint64 length) const
{
    // TODO: check if the it is legal to resize
    const QString len = QString::number(length / 512) + QStringLiteral("s");

    ExternalCommand cmd(report, QStringLiteral("lvm"), { QStringLiteral("pvresize"), QStringLiteral("--yes"), QStringLiteral("--setphysicalvolumesize"), len, deviceNode });
    return cmd.run(-1) && cmd.exitCode() == 0;
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
    Q_UNUSED(report);
    Q_UNUSED(deviceNode);
    Q_UNUSED(mountPoint);
    return false;
}

bool lvm2_pv::unmount(Report& report, const QString& deviceNode)
{
    Q_UNUSED(deviceNode);
    Q_UNUSED(report);
    return false;
}

bool lvm2_pv::canMount(const QString & deviceNode, const QString & mountPoint) const
{
    Q_UNUSED(mountPoint);
    QString rval = getpvField(QStringLiteral("pv_in_use"), deviceNode); // this field return "used" when in use otherwise empty string
    if (rval.isEmpty()) {
        return true;
    }
    return false;
}

bool lvm2_pv::canUnmount(const QString& deviceNode) const
{
    QString rval = getpvField(QStringLiteral("pv_in_use"), deviceNode);
    if (rval.isEmpty()) {
        return false;
    }
    return true;
}

qint64 lvm2_pv::getTotalPE(const QString& deviceNode) const
{
    QString val = getpvField(QStringLiteral("pv_pe_count"), deviceNode);
    return val.isEmpty() ? -1 : val.toLongLong();
}

qint64 lvm2_pv::getFreePE(const QString& deviceNode) const
{
    return getTotalPE(deviceNode) - getAllocatedPE(deviceNode);
}

qint64 lvm2_pv::getAllocatedPE(const QString& deviceNode) const
{
    QString val = getpvField(QStringLiteral("pv_pe_alloc_count"), deviceNode);
    return val.isEmpty() ? -1 : val.toLongLong();
}

qint64 lvm2_pv::getPVSize(const QString& deviceNode) const
{
    QString val = getpvField(QStringLiteral("pv_size"), deviceNode);
    return val.isEmpty() ? -1 : val.toLongLong();
}

qint64 lvm2_pv::getPESize(const QString& deviceNode) const
{
    QString val = getpvField(QStringLiteral("vg_extent_size"), deviceNode);
    return val.isEmpty() ? -1 : val.toLongLong();
}

QString  lvm2_pv::getpvField(const QString& fieldname, const QString& deviceNode)
{
    QStringList args = { QStringLiteral("pvs"),
                    QStringLiteral("--foreign"),
                    QStringLiteral("--readonly"),
                    QStringLiteral("--noheadings"),
                    QStringLiteral("--units"),
                    QStringLiteral("B"),
                    QStringLiteral("--nosuffix"),
                    QStringLiteral("--options"),
                    fieldname };
    if (!deviceNode.isEmpty()) {
        args << deviceNode;
    }
    ExternalCommand cmd(QStringLiteral("lvm"), args);
    if (cmd.run(-1) && cmd.exitCode() == 0) {
        return cmd.output().trimmed();
    }
    return QString();
}

QString lvm2_pv::getVGName(const QString& deviceNode)
{
    return getpvField(QStringLiteral("vg_name"), deviceNode);
}

QStringList lvm2_pv::getFreePV()
{
    QStringList rlist;

    QString output = getpvField(QStringLiteral("pv_name"));
    QStringList pvList = output.split(QStringLiteral("\n"), QString::SkipEmptyParts);
    foreach (QString pvnode, pvList) {
        if (!isUsed(pvnode.trimmed())) {
            rlist.append(pvnode.trimmed());
        }
    }

    return rlist;
}

QStringList lvm2_pv::getUsedPV(const QString& vgname)
{
    QStringList rlist;

    QString output = getpvField(QStringLiteral("pv_name"), vgname);
    QStringList pvList = output.split(QStringLiteral("\n"), QString::SkipEmptyParts);
    foreach (QString pvnode, pvList) {
        if (isUsed(pvnode.trimmed())) {
            rlist.append(pvnode.trimmed());
        }
    }

    return rlist;
}

bool lvm2_pv::isUsed(const QString& deviceNode)
{
    QString output = getpvField(QStringLiteral("pv_in_use"), deviceNode.trimmed());
    if (output.trimmed() == QStringLiteral("used")) {
        return true;
    }
    return false;
}

}
