/*
    SPDX-FileCopyrightText: 2012-2018 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2019 Yuri Chornoivan <yurchor@ukr.net>
    SPDX-FileCopyrightText: 2020 Arnaud Ferraris <arnaud.ferraris@collabora.com>
    SPDX-FileCopyrightText: 2020 Gaël PORTAY <gael.portay@collabora.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "fs/exfat.h"

#include "fs/clustersize.h"
#include "util/externalcommand.h"
#include "util/capacity.h"
#include "util/report.h"

#include <QRegularExpression>
#include <QString>
#include <QStringList>

namespace FS
{
FileSystem::CommandSupportType exfat::m_GetUsed = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType exfat::m_GetLabel = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType exfat::m_Create = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType exfat::m_Grow = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType exfat::m_Shrink = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType exfat::m_Move = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType exfat::m_Check = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType exfat::m_Copy = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType exfat::m_Backup = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType exfat::m_SetLabel = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType exfat::m_UpdateUUID = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType exfat::m_GetUUID = FileSystem::cmdSupportNone;
bool exfat::exfatUtils = false;

exfat::exfat(qint64 firstsector, qint64 lastsector, qint64 sectorsused, const QString& label, const QVariantMap& features) :
    FileSystem(firstsector, lastsector, sectorsused, label, features, FileSystem::Type::Exfat)
{
}

void exfat::init()
{
    // Check if we are using exfat-utils or exfatprogs
    exfatUtils = findExternal(QStringLiteral("mkexfatfs"));
    if (exfatUtils) {
        m_Create = cmdSupportFileSystem;
        m_Check = findExternal(QStringLiteral("fsck.exfat"), {}, 1) ? cmdSupportFileSystem : cmdSupportNone;
        m_SetLabel = findExternal(QStringLiteral("exfatlabel")) ? cmdSupportFileSystem : cmdSupportNone;
    }
    else {
        m_Create = findExternal(QStringLiteral("mkfs.exfat"), {}, 1) ? cmdSupportFileSystem : cmdSupportNone;
        m_Check = findExternal(QStringLiteral("fsck.exfat"), {}, 16) ? cmdSupportFileSystem : cmdSupportNone;
        m_SetLabel = findExternal(QStringLiteral("tune.exfat")) ? cmdSupportFileSystem : cmdSupportNone;
    }

    m_GetLabel = cmdSupportCore;
    m_UpdateUUID = cmdSupportNone;

    m_Copy = (m_Check != cmdSupportNone) ? cmdSupportCore : cmdSupportNone;
    m_Move = (m_Check != cmdSupportNone) ? cmdSupportCore : cmdSupportNone;

    m_GetLabel = cmdSupportCore;
    m_Backup = cmdSupportCore;
    m_GetUUID = cmdSupportCore;

    if (m_Create == cmdSupportFileSystem && !exfatUtils)
        addAvailableFeature(QStringLiteral("cluster-size"));
}

bool exfat::supportToolFound() const
{
    return
//          m_GetUsed != cmdSupportNone &&
        m_GetLabel != cmdSupportNone &&
        m_SetLabel != cmdSupportNone &&
        m_Create != cmdSupportNone &&
        m_Check != cmdSupportNone &&
//          m_UpdateUUID != cmdSupportNone &&
//          m_Grow != cmdSupportNone &&
//          m_Shrink != cmdSupportNone &&
        m_Copy != cmdSupportNone &&
        m_Move != cmdSupportNone &&
        m_Backup != cmdSupportNone &&
        m_GetUUID != cmdSupportNone;
}

FileSystem::SupportTool exfat::supportToolName() const
{
    return SupportTool(QStringLiteral("exfatprogs"), QUrl(QStringLiteral("https://github.com/exfatprogs/exfatprogs")));
}

qint64 exfat::maxCapacity() const
{
    return Capacity::unitFactor(Capacity::Unit::Byte, Capacity::Unit::EiB);
}

int exfat::maxLabelLength() const
{
    return 11;
}

static qint64 parseSize(const QString& s)
{
    const QRegularExpressionMatch match = QRegularExpression(QStringLiteral("(\\d+)\\s*([KMG]i?B|B)?")).match(s.trimmed());
    if (!match.hasMatch())
        return -1;

    qint64 value = match.captured(1).toLongLong();
    const QString unit = match.captured(2).toUpper();
    if (unit.startsWith(QLatin1Char('K')))
        value *= 1024;
    else if (unit.startsWith(QLatin1Char('M')))
        value *= 1024LL * 1024;
    else if (unit.startsWith(QLatin1Char('G')))
        value *= 1024LL * 1024 * 1024;

    return value;
}

void exfat::scan(const QString& deviceNode)
{
    setClusterSize(FS::ClusterSize::fromBootSector(deviceNode, type()));

    clearProperties();

    ExternalCommand cmd(exfatUtils ? QStringLiteral("dumpexfat") : QStringLiteral("dump.exfat"), { deviceNode });
    if (!cmd.run(-1) || cmd.exitCode() != 0)
        return;

    const QString output = cmd.output();
    QRegularExpression re;

    qint64 sectorSize = -1;
    qint64 clusterSize = -1;

    re.setPattern(QStringLiteral("Sector Size Bits:\\s*(\\d+)"));
    const QRegularExpressionMatch sectorBits = re.match(output);
    if (sectorBits.hasMatch()) {
        sectorSize = qint64(1) << sectorBits.captured(1).toLongLong();
        re.setPattern(QStringLiteral("Sector per Cluster bits:\\s*(\\d+)"));
        const QRegularExpressionMatch clusterBits = re.match(output);
        if (clusterBits.hasMatch())
            clusterSize = sectorSize << clusterBits.captured(1).toLongLong();
    } else {
        re.setPattern(QStringLiteral("Sector size\\s+([^\\n]+)"));
        const QRegularExpressionMatch sectorLine = re.match(output);
        if (sectorLine.hasMatch())
            sectorSize = parseSize(sectorLine.captured(1));
        re.setPattern(QStringLiteral("Cluster size\\s+([^\\n]+)"));
        const QRegularExpressionMatch clusterLine = re.match(output);
        if (clusterLine.hasMatch())
            clusterSize = parseSize(clusterLine.captured(1));
    }

    addProperty(QStringLiteral("sector-size"), sectorSize >= 0 ? QVariant(sectorSize) : QVariant(),
                FileSystemProperty::DisplayType::Bytes, FileSystemProperty::Group::UnitSizes);
    addProperty(QStringLiteral("cluster-size"), clusterSize >= 0 ? QVariant(clusterSize) : QVariant(),
                FileSystemProperty::DisplayType::Bytes, FileSystemProperty::Group::UnitSizes);

    re.setPattern(QStringLiteral("(?:File system version|FS version)\\s+([\\d.]+)"));
    const QRegularExpressionMatch version = re.match(output);
    if (version.hasMatch())
        addProperty(QStringLiteral("format-version"), QVariant(version.captured(1)),
                    FileSystemProperty::DisplayType::Text, FileSystemProperty::Group::Capabilities);
}

bool exfat::check(Report& report, const QString& deviceNode) const
{
    ExternalCommand cmd(report, QStringLiteral("fsck.exfat"));
    if (exfatUtils) {
        cmd.setArgs({ QStringLiteral("-a"), deviceNode });
    }
    else {
        cmd.setArgs({ QStringLiteral("--repair-yes"), QStringLiteral("--verbose"), deviceNode });
    }
    return cmd.run(-1) && cmd.exitCode() == 0;
}

bool exfat::create(Report& report, const QString& deviceNode)
{
    const QString clusterSizeError = validateClusterSizeFeature(length() * sectorSize());
    if (!clusterSizeError.isEmpty()) {
        report.line() << clusterSizeError;
        return false;
    }

    QStringList args;

    if (features().contains(QStringLiteral("cluster-size")))
        args << QStringLiteral("-c") << QString::number(features().value(QStringLiteral("cluster-size")).toLongLong());

    args << deviceNode;

    ExternalCommand cmd(report, QStringLiteral("mkfs.exfat"), args);
    return cmd.run(-1) && cmd.exitCode() == 0;
}

bool exfat::writeLabel(Report& report, const QString& deviceNode, const QString& newLabel)
{
    ExternalCommand cmd(report);
    if (exfatUtils) {
        cmd.setCommand(QStringLiteral("exfatlabel"));
        cmd.setArgs({ deviceNode, newLabel });
    }
    else {
        cmd.setCommand(QStringLiteral("tune.exfat"));
        cmd.setArgs({ deviceNode, QStringLiteral("-L"), newLabel });
    }

    return cmd.run(-1) && cmd.exitCode() == 0;
}

bool exfat::updateUUID(Report& report, const QString& deviceNode) const
{
    Q_UNUSED(report)
    Q_UNUSED(deviceNode)

    return false;
}
}
