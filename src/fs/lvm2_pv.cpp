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
    m_Create = findExternal(QStringLiteral("lvm")) ? cmdSupportFileSystem : cmdSupportNone;
    m_Check  = findExternal(QStringLiteral("lvm")) ? cmdSupportFileSystem : cmdSupportNone;
    m_Grow   = findExternal(QStringLiteral("lvm")) ? cmdSupportFileSystem : cmdSupportNone;
    m_Shrink = findExternal(QStringLiteral("lvm")) ? cmdSupportFileSystem : cmdSupportNone;

    m_GetLabel = cmdSupportCore;
    m_UpdateUUID = findExternal(QStringLiteral("lvm")) ? cmdSupportFileSystem : cmdSupportNone;

    m_Copy = cmdSupportNone; // Copying PV can confuse LVM
    m_Move = (m_Check != cmdSupportNone) ? cmdSupportCore : cmdSupportNone;

    m_GetLabel = cmdSupportNone;
    m_Backup = cmdSupportCore;
    m_GetUUID = cmdSupportCore;
}

bool lvm2_pv::supportToolFound() const
{
    return
//          m_GetUsed != cmdSupportNone &&
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
    // TODO: check if the it is legal to reize
    const QString len = QString::number(length / 512) + QStringLiteral("s");

    ExternalCommand cmd(report, QStringLiteral("lvm"), { QStringLiteral("pvresize"), QStringLiteral("--yes"), QStringLiteral("--setphysicalvolumesize"), len, deviceNode });
    return cmd.run(-1) && cmd.exitCode() == 0;
}

bool lvm2_pv::updateUUID(Report& report, const QString& deviceNode) const
{
    ExternalCommand cmd(report, QStringLiteral("lvm"), { QStringLiteral("pvchange"), QStringLiteral("--uuid"), deviceNode });
    return cmd.run(-1) && cmd.exitCode() == 0;
}

bool lvm2_pv::canMount(const QString & deviceNode, const QString & mountPoint) const
{
    Q_UNUSED(deviceNode)
    Q_UNUSED(mountPoint)
    return false;
}

QString lvm2_pv::getVGName(const QString& deviceNode) //PV node
{
    ExternalCommand cmd( QStringLiteral("lvm"),
                        { QStringLiteral("pvdisplay"), QStringLiteral("--verbose"), deviceNode });
    if (cmd.run(-1) && cmd.exitCode() == 0) {
        QRegularExpression re(QStringLiteral("VG Name\\h+(\\w+)"));
        QRegularExpressionMatch vgName = re.match(cmd.output());
        if (vgName.hasMatch())
            return vgName.captured(1);
    }
    return QString();
}

}
