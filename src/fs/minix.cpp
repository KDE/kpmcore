/*
    SPDX-FileCopyrightText: 2019 Shubham Jangra <aryan100jangid@gmail.com>
    SPDX-FileCopyrightText: 2020 Arnaud Ferraris <arnaud.ferraris@collabora.com>
    SPDX-FileCopyrightText: 2020 Gaël PORTAY <gael.portay@collabora.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/
 
#include "fs/minix.h"

#include "util/capacity.h"
#include "util/externalcommand.h"

#include <QString>

namespace FS
{
FileSystem::CommandSupportType minix::m_GetLabel = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType minix::m_GetUsed = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType minix::m_Shrink = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType minix::m_Move = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType minix::m_Check = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType minix::m_Create = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType minix::m_Copy = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType minix::m_Backup = FileSystem::cmdSupportNone;

minix::minix(qint64 firstsector, qint64 lastsector, qint64 sectorsused, const QString& label, const QVariantMap& features) :
    FileSystem(firstsector, lastsector, sectorsused, label, features, FileSystem::Type::Minix)
{
}

void minix::init()
{
    m_Check = findExternal(QStringLiteral("fsck.minix"), {}, 16) ? cmdSupportFileSystem : cmdSupportNone;
    m_Create = findExternal(QStringLiteral("mkfs.minix"), {}, 16) ? cmdSupportFileSystem : cmdSupportNone;
    m_Copy = (m_Check != cmdSupportNone) ? cmdSupportCore : cmdSupportNone;
    m_Move = (m_Check != cmdSupportNone) ? cmdSupportCore : cmdSupportNone;
    m_Backup = cmdSupportCore;
    m_GetLabel = cmdSupportCore;
}

bool minix::supportToolFound() const
{
    return m_GetLabel != cmdSupportNone &&
           m_Create != cmdSupportNone &&
           m_Check != cmdSupportNone &&
           m_Copy != cmdSupportNone &&
           m_Move != cmdSupportNone &&
           m_Backup != cmdSupportNone;
}

FileSystem::SupportTool minix::supportToolName() const
{
    return SupportTool(QStringLiteral("util-linux"), QUrl(QStringLiteral("https://www.kernel.org/pub/linux/utils/util-linux/")));
}

qint64 minix::maxCapacity() const
{
    return 4 * Capacity::unitFactor(Capacity::Unit::Byte, Capacity::Unit::GiB);
}

int minix::maxLabelLength() const
{
    return 63;
}

bool minix::check(Report& report, const QString& deviceNode) const
{
    ExternalCommand cmd(report, QStringLiteral("fsck.minix"), { deviceNode });
    return cmd.run(-1) && cmd.exitCode() == 0;
}

bool minix::create(Report& report, const QString& deviceNode)
{
    ExternalCommand cmd(report, QStringLiteral("mkfs.minix"), { QStringLiteral("-3"), deviceNode });
    return cmd.run(-1) && cmd.exitCode() == 0;
}

}
