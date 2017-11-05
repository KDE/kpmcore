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

#include "fs/btrfs.h"

#include "util/externalcommand.h"
#include "util/capacity.h"
#include "util/report.h"

#include <QRegularExpression>
#include <QString>
#include <QTemporaryDir>

#include <KLocalizedString>

namespace FS
{
FileSystem::CommandSupportType btrfs::m_GetUsed = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType btrfs::m_GetLabel = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType btrfs::m_Create = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType btrfs::m_Grow = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType btrfs::m_Shrink = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType btrfs::m_Move = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType btrfs::m_Check = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType btrfs::m_Copy = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType btrfs::m_Backup = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType btrfs::m_SetLabel = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType btrfs::m_UpdateUUID = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType btrfs::m_GetUUID = FileSystem::cmdSupportNone;

btrfs::btrfs(qint64 firstsector, qint64 lastsector, qint64 sectorsused, const QString& label) :
    FileSystem(firstsector, lastsector, sectorsused, label, FileSystem::Btrfs)
{
}

void btrfs::init()
{
    m_Create = findExternal(QStringLiteral("mkfs.btrfs")) ? cmdSupportFileSystem : cmdSupportNone;
    m_Check = findExternal(QStringLiteral("btrfsck"), QStringList(), 1) ? cmdSupportFileSystem : cmdSupportNone;
    m_Grow = (m_Check != cmdSupportNone && findExternal(QStringLiteral("btrfs"))) ? cmdSupportFileSystem : cmdSupportNone;
    m_GetUsed = findExternal(QStringLiteral("btrfs")) ? cmdSupportFileSystem : cmdSupportNone;
    m_Shrink = (m_Grow != cmdSupportNone && m_GetUsed != cmdSupportNone) ? cmdSupportFileSystem : cmdSupportNone;

    m_SetLabel = findExternal(QStringLiteral("btrfs")) ? cmdSupportFileSystem : cmdSupportNone;
    m_UpdateUUID = findExternal(QStringLiteral("btrfstune")) ? cmdSupportFileSystem : cmdSupportNone;

    m_Copy = (m_Check != cmdSupportNone) ? cmdSupportCore : cmdSupportNone;
    m_Move = (m_Check != cmdSupportNone) ? cmdSupportCore : cmdSupportNone;

    m_GetLabel = cmdSupportCore;
    m_Backup = cmdSupportCore;
    m_GetUUID = cmdSupportCore;
}

bool btrfs::supportToolFound() const
{
    return
        m_GetUsed != cmdSupportNone &&
        m_GetLabel != cmdSupportNone &&
        m_SetLabel != cmdSupportNone &&
        m_Create != cmdSupportNone &&
        m_Check != cmdSupportNone &&
//          m_UpdateUUID != cmdSupportNone &&
        m_Grow != cmdSupportNone &&
        m_Shrink != cmdSupportNone &&
        m_Copy != cmdSupportNone &&
        m_Move != cmdSupportNone &&
        m_Backup != cmdSupportNone &&
        m_GetUUID != cmdSupportNone;
}

FileSystem::SupportTool btrfs::supportToolName() const
{
    return SupportTool(QStringLiteral("btrfs-tools"), QUrl(QStringLiteral("http://btrfs.wiki.kernel.org/")));
}

qint64 btrfs::minCapacity() const
{
    return 256 * Capacity::unitFactor(Capacity::Byte, Capacity::MiB);
}

qint64 btrfs::maxCapacity() const
{
    return Capacity::unitFactor(Capacity::Byte, Capacity::EiB);
}

int btrfs::maxLabelLength() const
{
    return 255;
}

qint64 btrfs::readUsedCapacity(const QString& deviceNode) const
{
    ExternalCommand cmd(QStringLiteral("btrfs"),
                        { QStringLiteral("filesystem"), QStringLiteral("show"), QStringLiteral("--raw"), deviceNode });

    if (cmd.run(-1) && cmd.exitCode() == 0) {
        QRegularExpression re(QStringLiteral(" used (\\d+) path ") + deviceNode);
        QRegularExpressionMatch reBytesUsed = re.match(cmd.output());

        if (reBytesUsed.hasMatch())
            return reBytesUsed.captured(1).toLongLong();
    }

    return -1;
}

bool btrfs::check(Report& report, const QString& deviceNode) const
{
    ExternalCommand cmd(report, QStringLiteral("btrfsck"), { deviceNode });
    return cmd.run(-1) && cmd.exitCode() == 0;
}

bool btrfs::create(Report& report, const QString& deviceNode)
{
    ExternalCommand cmd(report, QStringLiteral("mkfs.btrfs"), { QStringLiteral("--force"), deviceNode });
    return cmd.run(-1) && cmd.exitCode() == 0;
}

bool btrfs::resize(Report& report, const QString& deviceNode, qint64 length) const
{
    QTemporaryDir tempDir;
    if (!tempDir.isValid()) {
        report.line() << xi18nc("@info:progress", "Resizing Btrfs file system on partition <filename>%1</filename> failed: Could not create temp dir.", deviceNode);
        return false;
    }

    bool rval = false;

    ExternalCommand mountCmd(report, QStringLiteral("mount"),
                             { QStringLiteral("--verbose"),  QStringLiteral("--types"), QStringLiteral("btrfs"), deviceNode, tempDir.path() });

    if (mountCmd.run(-1) && mountCmd.exitCode() == 0) {
        ExternalCommand resizeCmd(report, QStringLiteral("btrfs"),
                                  { QStringLiteral("filesystem"), QStringLiteral("resize"), QString::number(length), tempDir.path() });

        if (resizeCmd.run(-1) && resizeCmd.exitCode() == 0)
            rval = true;
        else
            report.line() << xi18nc("@info:progress", "Resizing Btrfs file system on partition <filename>%1</filename> failed: btrfs file system resize failed.", deviceNode);

        ExternalCommand unmountCmd(report, QStringLiteral("umount"), { tempDir.path() });

        if (!unmountCmd.run(-1) && unmountCmd.exitCode() == 0)
            report.line() << xi18nc("@info:progress", "<warning>Resizing Btrfs file system on partition <filename>%1</filename>: Unmount failed.</warning>", deviceNode);
    } else
        report.line() << xi18nc("@info:progress", "Resizing Btrfs file system on partition <filename>%1</filename> failed: Initial mount failed.", deviceNode);

    return rval;
}

bool btrfs::resizeOnline(Report& report, const QString& deviceNode, const QString& mountPoint, qint64 length) const
{
    ExternalCommand resizeCmd(report, QStringLiteral("btrfs"),
                              { QStringLiteral("filesystem"), QStringLiteral("resize"), QString::number(length), mountPoint });

    if (resizeCmd.run(-1) && resizeCmd.exitCode() == 0)
        return true;

    report.line() << xi18nc("@info:progress", "Resizing Btrfs file system on partition <filename>%1</filename> failed: btrfs file system resize failed.", deviceNode);
    return false;
}

bool btrfs::writeLabel(Report& report, const QString& deviceNode, const QString& newLabel)
{
    ExternalCommand cmd(report, QStringLiteral("btrfs"), { QStringLiteral("filesystem"), QStringLiteral("label"), deviceNode, newLabel });
    return cmd.run(-1) && cmd.exitCode() == 0;
}

bool btrfs::writeLabelOnline(Report& report, const QString& deviceNode, const QString& mountPoint, const QString& newLabel)
{
    Q_UNUSED(deviceNode)
    ExternalCommand cmd(report, QStringLiteral("btrfs"), { QStringLiteral("filesystem"), QStringLiteral("label"), mountPoint, newLabel });
    return cmd.run(-1) && cmd.exitCode() == 0;
}

bool btrfs::updateUUID(Report& report, const QString& deviceNode) const
{
    ExternalCommand cmd(report, QStringLiteral("btrfstune"), { QStringLiteral("-f"), QStringLiteral("-u"), deviceNode });
    return cmd.run(-1) && cmd.exitCode() == 0;
}
}
