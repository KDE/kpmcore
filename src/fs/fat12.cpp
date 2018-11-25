/*************************************************************************
 *  Copyright (C) 2008,2009,2011 by Volker Lanz <vl@fidra.de>            *
 *  Copyright (C) 2017 by Andrius Štikonas <andrius@stikonas.eu>         *
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

#include "fs/fat12.h"

#include "util/externalcommand.h"
#include "util/capacity.h"
#include "util/report.h"

#include <KLocalizedString>

#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QString>
#include <QStringList>

#include <ctime>

namespace FS
{
FileSystem::CommandSupportType fat12::m_GetUsed = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType fat12::m_GetLabel = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType fat12::m_SetLabel = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType fat12::m_Create = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType fat12::m_Grow = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType fat12::m_Shrink = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType fat12::m_Move = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType fat12::m_Check = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType fat12::m_Copy = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType fat12::m_Backup = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType fat12::m_UpdateUUID = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType fat12::m_GetUUID = FileSystem::cmdSupportNone;

fat12::fat12(qint64 firstsector, qint64 lastsector, qint64 sectorsused, const QString& label, FileSystem::Type t) :
    FileSystem(firstsector, lastsector, sectorsused, label, t)
{
}

void fat12::init()
{
    m_Create = m_GetUsed = m_Check = findExternal(QStringLiteral("mkfs.fat"), {}, 1) ? cmdSupportFileSystem : cmdSupportNone;
    m_GetLabel = cmdSupportCore;
    m_SetLabel = findExternal(QStringLiteral("fatlabel")) ? cmdSupportFileSystem : cmdSupportNone;
    m_Move = cmdSupportCore;
    m_Copy = cmdSupportCore;
    m_Backup = cmdSupportCore;
    m_UpdateUUID = findExternal(QStringLiteral("dd")) ? cmdSupportFileSystem : cmdSupportNone;
    m_GetUUID = cmdSupportCore;
}

bool fat12::supportToolFound() const
{
    return
        m_GetUsed != cmdSupportNone &&
        m_GetLabel != cmdSupportNone &&
        m_SetLabel != cmdSupportNone &&
        m_Create != cmdSupportNone &&
        m_Check != cmdSupportNone &&
        m_UpdateUUID != cmdSupportNone &&
        m_Copy != cmdSupportNone &&
        m_Move != cmdSupportNone &&
        m_Backup != cmdSupportNone &&
        m_GetUUID != cmdSupportNone;
}

FileSystem::SupportTool fat12::supportToolName() const
{
    // also, dd for updating the UUID, but let's assume it's there ;-)
    return SupportTool(QStringLiteral("dosfstools"), QUrl(QStringLiteral("http://www.daniel-baumann.ch/software/dosfstools/")));
}


qint64 fat12::minCapacity() const
{
    return 1 * Capacity::unitFactor(Capacity::Unit::Byte, Capacity::Unit::MiB);
}

qint64 fat12::maxCapacity() const
{
    return 255 * Capacity::unitFactor(Capacity::Unit::Byte, Capacity::Unit::MiB);
}

int fat12::maxLabelLength() const
{
    return 11;
}

QValidator* fat12::labelValidator(QObject *parent) const
{
    QRegularExpressionValidator *m_LabelValidator = new QRegularExpressionValidator(parent);
    m_LabelValidator->setRegularExpression(QRegularExpression(QStringLiteral(R"(^[^\x{0000}-\x{001F}\x{007F}-\x{FFFF}*?.,;:\/\\|+=<>\[\]"]*$)")));
    return m_LabelValidator;
}

qint64 fat12::readUsedCapacity(const QString& deviceNode) const
{
    ExternalCommand cmd(QStringLiteral("fsck.fat"), { QStringLiteral("-n"), QStringLiteral("-v"), deviceNode });

    // Exit code 1 is returned when FAT dirty bit is set
    if (cmd.run(-1) && (cmd.exitCode() == 0 || cmd.exitCode() == 1)) {
        qint64 usedClusters = -1;
        QRegularExpression re(QStringLiteral("files, (\\d+)/\\d+ "));
        QRegularExpressionMatch reClusters = re.match(cmd.output());

        if (reClusters.hasMatch())
            usedClusters = reClusters.captured(1).toLongLong();

        qint64 clusterSize = -1;

        re.setPattern(QStringLiteral("(\\d+) bytes per cluster"));
        QRegularExpressionMatch reClusterSize = re.match(cmd.output());

        if (reClusterSize.hasMatch())
            clusterSize = reClusterSize.captured(1).toLongLong();

        if (usedClusters > -1 && clusterSize > -1)
            return usedClusters * clusterSize;
    }

    return -1;
}

bool fat12::writeLabel(Report& report, const QString& deviceNode, const QString& newLabel)
{
    report.line() << xi18nc("@info:progress", "Setting label for partition <filename>%1</filename> to %2", deviceNode, newLabel.toUpper());

    ExternalCommand cmd(report, QStringLiteral("fatlabel"), { deviceNode, newLabel.toUpper() });
    return cmd.run(-1) && cmd.exitCode() == 0;
}

bool fat12::check(Report& report, const QString& deviceNode) const
{
    ExternalCommand cmd(report, QStringLiteral("fsck.fat"), { QStringLiteral("-a"), QStringLiteral("-w"), QStringLiteral("-v"), deviceNode });
    return cmd.run(-1) && cmd.exitCode() == 0;
}

bool fat12::create(Report& report, const QString& deviceNode)
{
    ExternalCommand cmd(report, QStringLiteral("mkfs.fat"), { QStringLiteral("-F12"), QStringLiteral("-I"), QStringLiteral("-v"), deviceNode });
    return cmd.run(-1) && cmd.exitCode() == 0;
}

bool fat12::updateUUID(Report& report, const QString& deviceNode) const
{
    long int t = time(nullptr);

    char uuid[4];
    for (auto &u : uuid) {
        u = static_cast<char>(t & 0xff);
        t >>= 8;
    }

    ExternalCommand cmd;
    return cmd.writeData(report, QByteArray(uuid, sizeof(uuid)), deviceNode, 39);
}
}
