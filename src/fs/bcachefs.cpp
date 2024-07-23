/*
    SPDX-FileCopyrightText: 2024 Andrius Štikonas <andrius@stikonas.eu>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "fs/bcachefs.h"

#include "util/externalcommand.h"
#include "util/capacity.h"
#include "util/report.h"

#include <QRegularExpression>
#include <QString>
#include <QStringList>
#include <QTemporaryDir>

#include <KLocalizedString>

namespace FS
{
FileSystem::CommandSupportType bcachefs::m_GetUsed = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType bcachefs::m_GetLabel = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType bcachefs::m_Create = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType bcachefs::m_Grow = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType bcachefs::m_Shrink = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType bcachefs::m_Move = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType bcachefs::m_Check = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType bcachefs::m_Copy = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType bcachefs::m_Backup = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType bcachefs::m_SetLabel = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType bcachefs::m_UpdateUUID = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType bcachefs::m_GetUUID = FileSystem::cmdSupportNone;

bcachefs::bcachefs(qint64 firstsector, qint64 lastsector, qint64 sectorsused, const QString& label, const QVariantMap& features) :
    FileSystem(firstsector, lastsector, sectorsused, label, features, FileSystem::Type::Bcachefs)
{
}

void bcachefs::init()
{
    m_Create = findExternal(QStringLiteral("bcachefs")) ? cmdSupportFileSystem : cmdSupportNone;
    m_SetLabel = m_Grow = m_Check = m_Create;
    m_Shrink = cmdSupportNone;

    m_Copy = (m_Check != cmdSupportNone) ? cmdSupportCore : cmdSupportNone;
    m_Move = (m_Check != cmdSupportNone) ? cmdSupportCore : cmdSupportNone;

    m_GetUsed = cmdSupportCore;

    m_GetLabel = cmdSupportCore;
    m_Backup = cmdSupportCore;
    m_GetUUID = cmdSupportCore;
}

bool bcachefs::supportToolFound() const
{
    return
        m_GetUsed != cmdSupportNone &&
        m_GetLabel != cmdSupportNone &&
        m_SetLabel != cmdSupportNone &&
        m_Create != cmdSupportNone &&
        m_Check != cmdSupportNone &&
//        m_UpdateUUID != cmdSupportNone &&
        m_Grow != cmdSupportNone &&
//        m_Shrink != cmdSupportNone &&
        m_Copy != cmdSupportNone &&
        m_Move != cmdSupportNone &&
        m_Backup != cmdSupportNone &&
        m_GetUUID != cmdSupportNone;
}

FileSystem::SupportTool bcachefs::supportToolName() const
{
    return SupportTool(QStringLiteral("bcachefs"), QUrl(QStringLiteral("https://bcachefs.org/")));
}

qint64 bcachefs::minCapacity() const
{
    return 32 * Capacity::unitFactor(Capacity::Unit::Byte, Capacity::Unit::MiB);
}

qint64 bcachefs::maxCapacity() const
{
    return Capacity::unitFactor(Capacity::Unit::Byte, Capacity::Unit::EiB);
}

int bcachefs::maxLabelLength() const
{
    return 255;
}

// qint64 bcachefs::readUsedCapacity(const QString& deviceNode) const
// {
//     return -1;
// }

bool bcachefs::check(Report& report, const QString& deviceNode) const
{
    ExternalCommand cmd(report, QStringLiteral("bcachefs"), { QStringLiteral("fsck"), QStringLiteral("-f"), QStringLiteral("-y"), deviceNode });
    return cmd.run(-1) && cmd.exitCode() == 0;
}

bool bcachefs::create(Report& report, const QString& deviceNode)
{
    ExternalCommand cmd(report, QStringLiteral("bcachefs"), { QStringLiteral("format"), QStringLiteral("--force"),  deviceNode });
    return cmd.run(-1) && cmd.exitCode() == 0;
}

bool bcachefs::createWithLabel(Report& report, const QString& deviceNode, const QString& label)
{
    ExternalCommand cmd(report, QStringLiteral("bcachefs"), { QStringLiteral("format"), QStringLiteral("--force"),  QStringLiteral("--fs_label="), label, deviceNode });
    return cmd.run(-1) && cmd.exitCode() == 0;
}

bool bcachefs::resize(Report& report, const QString& deviceNode, qint64 length) const
{
    QTemporaryDir tempDir;
    if (!tempDir.isValid()) {
        report.line() << xi18nc("@info:progress", "Resizing Bcachefs file system on partition <filename>%1</filename> failed: Could not create temp dir.", deviceNode);
        return false;
    }

    bool rval = false;

    ExternalCommand mountCmd(report, QStringLiteral("mount"),
                             { QStringLiteral("--verbose"),  QStringLiteral("--types"), QStringLiteral("bcachefs"), deviceNode, tempDir.path() });

    if (mountCmd.run(-1) && mountCmd.exitCode() == 0) {
        ExternalCommand resizeCmd(report, QStringLiteral("bcachefs"),
                                  { QStringLiteral("device"), QStringLiteral("resize"), deviceNode, QString::number(length) });

        if (resizeCmd.run(-1) && resizeCmd.exitCode() == 0)
            rval = true;
        else
            report.line() << xi18nc("@info:progress", "Resizing Bcachefs file system on partition <filename>%1</filename> failed: bcachefs device resize command failed.", deviceNode);

        ExternalCommand unmountCmd(report, QStringLiteral("umount"), { tempDir.path() });

        if (!unmountCmd.run(-1) && unmountCmd.exitCode() == 0)
            report.line() << xi18nc("@info:progress", "<warning>Resizing Bcachefs file system on partition <filename>%1</filename>: Unmount failed.</warning>", deviceNode);
    }
    else
        report.line() << xi18nc("@info:progress", "Resizing Bcachefs file system on partition <filename>%1</filename> failed: Initial mount failed.", deviceNode);

    return rval;
}

bool bcachefs::resizeOnline(Report& report, const QString& deviceNode, const QString& mountPoint, qint64 length) const
{
    Q_UNUSED(mountPoint)
    ExternalCommand resizeCmd(report, QStringLiteral("bcachefs"),
                              { QStringLiteral("device"), QStringLiteral("resize"), deviceNode, QString::number(length) });

    if (resizeCmd.run(-1) && resizeCmd.exitCode() == 0)
        return true;

    report.line() << xi18nc("@info:progress", "Resizing Bcachefs file system on partition <filename>%1</filename> failed: bcachefs device resize command failed.", deviceNode);
    return false;
}
}
