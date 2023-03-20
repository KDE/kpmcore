/*
    SPDX-FileCopyrightText: 2008-2011 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2013-2018 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2015 Teo Mrnjavac <teo@kde.org>
    SPDX-FileCopyrightText: 2017 Pali Rohár <pali.rohar@gmail.com>
    SPDX-FileCopyrightText: 2020 Arnaud Ferraris <arnaud.ferraris@collabora.com>
    SPDX-FileCopyrightText: 2020 Gaël PORTAY <gael.portay@collabora.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "fs/fat12.h"

#include "util/externalcommand.h"
#include "util/capacity.h"
#include "util/report.h"

#include <KLocalizedString>

#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QString>
#include <QStringList>

#include <QDebug>
#include <QtMath>

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

fat12::fat12(qint64 firstsector, qint64 lastsector, qint64 sectorsused, const QString& label, const QVariantMap& features, FileSystem::Type t) :
    FileSystem(firstsector, lastsector, sectorsused, label, features, t)
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
    m_UpdateUUID = cmdSupportCore;
    m_GetUUID = cmdSupportCore;

    if (m_Create == cmdSupportFileSystem) {
        addAvailableFeature(QStringLiteral("sector-size"));
        addAvailableFeature(QStringLiteral("sectors-per-cluster"));
    }
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

    const QString label = newLabel.isEmpty() ? QStringLiteral("-r") : newLabel.toUpper();
    ExternalCommand cmd(report, QStringLiteral("fatlabel"), { deviceNode, label });
    return cmd.run(-1) && cmd.exitCode() == 0;
}

bool fat12::check(Report& report, const QString& deviceNode) const
{
    ExternalCommand cmd(report, QStringLiteral("fsck.fat"), { QStringLiteral("-a"), QStringLiteral("-w"), QStringLiteral("-v"), deviceNode });
    return cmd.run(-1) && cmd.exitCode() == 0;
}

bool fat12::create(Report& report, const QString& deviceNode)
{
    return createWithFatSize(report, deviceNode, 12);
}

bool fat12::createWithFatSize(Report &report, const QString& deviceNode, int fatSize)
{
    QStringList args = QStringList();

    if (fatSize != 12 && fatSize != 16 && fatSize != 32)
        return false;

    for (const auto& k : this->features().keys()) {
	const auto& v = this->features().value(k);
        if (k == QStringLiteral("sector-size")) {
            quint32 sectorSize = v.toInt();

            /* sectorSize has to be a power of 2 between 512 and 32768 */
            if (sectorSize >= 512 && sectorSize <= 32768 && sectorSize == qNextPowerOfTwo(sectorSize - 1))
                args << QStringLiteral("-S%1").arg(sectorSize);
            else
                qWarning() << QStringLiteral("FAT sector size %1 is invalid, using default").arg(sectorSize);
        } else if (k == QStringLiteral("sectors-per-cluster")) {
            quint32 sectorsPerCluster = v.toInt();

            /* sectorsPerCluster has to be a power of 2 between 2 and 128 */
            if (sectorsPerCluster <= 128 && sectorsPerCluster == qNextPowerOfTwo(sectorsPerCluster - 1))
                args << QStringLiteral("-s%1").arg(sectorsPerCluster);
            else
                qWarning() << QStringLiteral("FAT sector size %1 is invalid, using default").arg(sectorsPerCluster);
        }
    }
    args << QStringLiteral("-F%1").arg(fatSize) << QStringLiteral("-I") << QStringLiteral("-v") << deviceNode;

    ExternalCommand cmd(report, QStringLiteral("mkfs.fat"), args);
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
