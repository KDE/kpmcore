/*
    SPDX-FileCopyrightText: 2016-2018 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2020 Arnaud Ferraris <arnaud.ferraris@collabora.com>
    SPDX-FileCopyrightText: 2020 Gaël PORTAY <gael.portay@collabora.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

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

f2fs::f2fs(qint64 firstsector, qint64 lastsector, qint64 sectorsused, const QString& label, const QVariantMap& features) :
    FileSystem(firstsector, lastsector, sectorsused, label, features, FileSystem::Type::F2fs)
{
}

void f2fs::init()
{
    m_Create = findExternal(QStringLiteral("mkfs.f2fs")) ? cmdSupportFileSystem : cmdSupportNone;
    m_Check = findExternal(QStringLiteral("fsck.f2fs")) ? cmdSupportFileSystem : cmdSupportNone;
    m_GetLabel = cmdSupportCore;
    m_SetLabel = findExternal(QStringLiteral("f2fslabel")) ? cmdSupportFileSystem : cmdSupportNone;
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
        m_SetLabel != cmdSupportNone &&
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

bool f2fs::writeLabel(Report& report, const QString& deviceNode, const QString& newLabel)
{
    ExternalCommand cmd(report, QStringLiteral("f2fslabel"), { deviceNode, newLabel });
    return cmd.run(-1) && cmd.exitCode() == 0;
}

bool f2fs::create(Report& report, const QString& deviceNode)
{
    ExternalCommand cmd(report, QStringLiteral("mkfs.f2fs"), { QStringLiteral("-f"), deviceNode });
    return cmd.run(-1) && cmd.exitCode() == 0;
}

bool f2fs::resize(Report& report, const QString& deviceNode, qint64 length) const
{
    Q_UNUSED(length)
    ExternalCommand cmd(report, QStringLiteral("resize.f2fs"), { deviceNode });
    return cmd.run(-1) && cmd.exitCode() == 0;
}

}
