/*************************************************************************
 *  Copyright (C) 2008,2011 by Volker Lanz <vl@fidra.de>                 *
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

#include "fs/hfsplus.h"

#include "util/externalcommand.h"
#include "util/capacity.h"

#include <QStringList>

namespace FS
{
FileSystem::CommandSupportType hfsplus::m_GetLabel = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType hfsplus::m_GetUsed = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType hfsplus::m_Shrink = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType hfsplus::m_Move = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType hfsplus::m_Check = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType hfsplus::m_Create = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType hfsplus::m_Copy = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType hfsplus::m_Backup = FileSystem::cmdSupportNone;

hfsplus::hfsplus(qint64 firstsector, qint64 lastsector, qint64 sectorsused, const QString& label) :
    FileSystem(firstsector, lastsector, sectorsused, label, FileSystem::Type::HfsPlus)
{
}

void hfsplus::init()
{
    m_Check = findExternal(QStringLiteral("fsck_hfs")) ? cmdSupportFileSystem : cmdSupportNone;
    m_Create = findExternal(QStringLiteral("newfs_hfs")) ? cmdSupportFileSystem : cmdSupportNone;
    m_Copy = (m_Check != cmdSupportNone) ? cmdSupportCore : cmdSupportNone;
    m_Move = (m_Check != cmdSupportNone) ? cmdSupportCore : cmdSupportNone;
    m_Backup = cmdSupportCore;
    m_GetLabel = cmdSupportCore;
}

bool hfsplus::supportToolFound() const
{
    return
//          m_GetUsed != cmdSupportNone &&
        m_GetLabel != cmdSupportNone &&
//          m_SetLabel != cmdSupportNone &&
        m_Create != cmdSupportNone &&
        m_Check != cmdSupportNone &&
//          m_UpdateUUID != cmdSupportNone &&
//          m_Grow != cmdSupportNone &&
        m_Shrink != cmdSupportNone &&
        m_Copy != cmdSupportNone &&
        m_Move != cmdSupportNone &&
        m_Backup != cmdSupportNone;
//         m_GetUUID != cmdSupportNone;
}

FileSystem::SupportTool hfsplus::supportToolName() const
{
    return SupportTool(QStringLiteral("diskdev_cmds"), QUrl(QStringLiteral("http://opendarwin.org")));
}

qint64 hfsplus::maxCapacity() const
{
    return Capacity::unitFactor(Capacity::Unit::Byte, Capacity::Unit::EiB);
}

int hfsplus::maxLabelLength() const
{
    return 63;
}

bool hfsplus::check(Report& report, const QString& deviceNode) const
{
    ExternalCommand cmd(report, QStringLiteral("fsck.hfsplus"), { deviceNode });
    return cmd.run(-1) && cmd.exitCode() == 0;
}

bool hfsplus::create(Report& report, const QString& deviceNode)
{
    ExternalCommand cmd(report, QStringLiteral("mkfs.hfsplus"), { deviceNode });
    return cmd.run(-1) && cmd.exitCode() == 0;
}

}
