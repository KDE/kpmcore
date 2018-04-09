/*************************************************************************
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

#include "fs/f2fs.h"

#include "util/externalcommand.h"
#include "util/capacity.h"
#include "util/report.h"

#include <cmath>

#include <QString>
#include <QTemporaryDir>
#include <QUuid>

#include <KLocalizedString>

namespace FS
{
FileSystem::CommandSupportType f2fs::m_GetUsed = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType f2fs::m_GetLabel = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType f2fs::m_Create = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType f2fs::m_Grow = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType f2fs::m_Shrink = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType f2fs::m_Move = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType f2fs::m_Check = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType f2fs::m_Copy = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType f2fs::m_Backup = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType f2fs::m_SetLabel = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType f2fs::m_UpdateUUID = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType f2fs::m_GetUUID = FileSystem::cmdSupportNone;
bool f2fs::oldVersion = false; // 1.8.x or older

f2fs::f2fs(qint64 firstsector, qint64 lastsector, qint64 sectorsused, const QString& label) :
    FileSystem(firstsector, lastsector, sectorsused, label, FileSystem::Type::F2fs)
{
}

void f2fs::init()
{
    m_Create = findExternal(QStringLiteral("mkfs.f2fs")) ? cmdSupportFileSystem : cmdSupportNone;
    m_Check = findExternal(QStringLiteral("fsck.f2fs")) ? cmdSupportFileSystem : cmdSupportNone;

    if (m_Create == cmdSupportFileSystem) {
        ExternalCommand cmd(QStringLiteral("mkfs.f2fs"), {});
        oldVersion = cmd.run(-1) && !cmd.output().contains(QStringLiteral("-f"));
    }

    m_GetLabel = cmdSupportCore;
//     m_SetLabel = findExternal(QStringLiteral("nilfs-tune")) ? cmdSupportFileSystem : cmdSupportNone;
//     m_UpdateUUID = findExternal(QStringLiteral("nilfs-tune")) ? cmdSupportFileSystem : cmdSupportNone;

    m_Grow = (m_Check != cmdSupportNone && findExternal(QStringLiteral("resize.f2fs"))) ? cmdSupportFileSystem : cmdSupportNone;
//     m_GetUsed = findExternal(QStringLiteral("nilfs-tune")) ? cmdSupportFileSystem : cmdSupportNone;
//     m_Shrink = (m_Grow != cmdSupportNone && m_GetUsed != cmdSupportNone) ? cmdSupportFileSystem : cmdSupportNone;

    m_Copy = (m_Check != cmdSupportNone) ? cmdSupportCore : cmdSupportNone;
    m_Move = (m_Check != cmdSupportNone) ? cmdSupportCore : cmdSupportNone;

    m_GetLabel = cmdSupportCore;
    m_Backup = cmdSupportCore;
    m_GetUUID = cmdSupportCore;
}

bool f2fs::supportToolFound() const
{
    return
//         m_GetUsed != cmdSupportNone &&
        m_GetLabel != cmdSupportNone &&
//         m_SetLabel != cmdSupportNone &&
        m_Create != cmdSupportNone &&
        m_Check != cmdSupportNone &&
//         m_UpdateUUID != cmdSupportNone &&
        m_Grow != cmdSupportNone &&
//         m_Shrink != cmdSupportNone &&
        m_Copy != cmdSupportNone &&
        m_Move != cmdSupportNone &&
        m_Backup != cmdSupportNone &&
        m_GetUUID != cmdSupportNone;
}

FileSystem::SupportTool f2fs::supportToolName() const
{
    return SupportTool(QStringLiteral("f2fs-tools"), QUrl(QStringLiteral("https://git.kernel.org/cgit/linux/kernel/git/jaegeuk/f2fs-tools.git")));
}

qint64 f2fs::minCapacity() const
{
    return 30 * Capacity::unitFactor(Capacity::Unit::Byte, Capacity::Unit::MiB);
}

qint64 f2fs::maxCapacity() const
{
    return 16 * Capacity::unitFactor(Capacity::Unit::Byte, Capacity::Unit::TiB);
}

int f2fs::maxLabelLength() const
{
    return 80;
}

bool f2fs::check(Report& report, const QString& deviceNode) const
{
    ExternalCommand cmd(report, QStringLiteral("fsck.f2fs"), { deviceNode });
    return cmd.run(-1) && cmd.exitCode() == 0;
}

bool f2fs::create(Report& report, const QString& deviceNode)
{
    return createWithLabel(report, deviceNode, QString());
}

bool f2fs::createWithLabel(Report& report, const QString& deviceNode, const QString& label)
{
    QStringList args;
    if (oldVersion)
        args << QStringLiteral("-l") << label << deviceNode;
    else
        args << QStringLiteral("-f") << QStringLiteral("-l") << label << deviceNode;
    ExternalCommand cmd(report, QStringLiteral("mkfs.f2fs"), args);
    return cmd.run(-1) && cmd.exitCode() == 0;
}

bool f2fs::resize(Report& report, const QString& deviceNode, qint64 length) const
{
    Q_UNUSED(length)
    ExternalCommand cmd(report, QStringLiteral("resize.f2fs"), { deviceNode });
    return cmd.run(-1) && cmd.exitCode() == 0;
}

}
