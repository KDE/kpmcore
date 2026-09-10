/*
    SPDX-FileCopyrightText: 2008-2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2012-2018 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2020 Arnaud Ferraris <arnaud.ferraris@collabora.com>
    SPDX-FileCopyrightText: 2020 Gaël PORTAY <gael.portay@collabora.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "fs/ext2.h"

#include "util/externalcommand.h"
#include "util/capacity.h"

#include <QRegularExpression>
#include <QString>

namespace FS
{
FileSystem::CommandSupportType ext2::m_GetUsed = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType ext2::m_GetLabel = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType ext2::m_Create = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType ext2::m_Grow = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType ext2::m_Shrink = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType ext2::m_Move = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType ext2::m_Check = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType ext2::m_Copy = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType ext2::m_Backup = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType ext2::m_SetLabel = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType ext2::m_UpdateUUID = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType ext2::m_GetUUID = FileSystem::cmdSupportNone;

ext2::ext2(qint64 firstsector, qint64 lastsector, qint64 sectorsused, const QString& label, const QVariantMap& features, FileSystem::Type t) :
    FileSystem(firstsector, lastsector, sectorsused, label, features, t)
{
}

void ext2::init()
{
    m_GetUsed = findExternal(QStringLiteral("dumpe2fs")) ? cmdSupportFileSystem : cmdSupportNone;
    m_GetLabel = cmdSupportCore;
    m_SetLabel = findExternal(QStringLiteral("e2label")) ? cmdSupportFileSystem : cmdSupportNone;
    m_Create = findExternal(QStringLiteral("mkfs.ext2")) ? cmdSupportFileSystem : cmdSupportNone;
    m_Check = findExternal(QStringLiteral("e2fsck"), { QStringLiteral("-V") }) ? cmdSupportFileSystem : cmdSupportNone;
    m_UpdateUUID = findExternal(QStringLiteral("tune2fs")) ? cmdSupportFileSystem : cmdSupportNone;
    m_Grow = (m_Check != cmdSupportNone && findExternal(QStringLiteral("resize2fs"))) ? cmdSupportFileSystem : cmdSupportNone;
    m_Shrink = (m_Grow != cmdSupportNone && m_GetUsed) != cmdSupportNone ? cmdSupportFileSystem : cmdSupportNone;
    m_Copy = (m_Check != cmdSupportNone) ? cmdSupportCore : cmdSupportNone;
    m_Move = (m_Check != cmdSupportNone) ? cmdSupportCore : cmdSupportNone;
    m_Backup = cmdSupportCore;
    m_GetUUID = cmdSupportCore;

    if (m_Create == cmdSupportFileSystem) {
        addAvailableFeature(QStringLiteral("64bit"));
        addAvailableFeature(QStringLiteral("bigalloc"));
        addAvailableFeature(QStringLiteral("casefold"));
        addAvailableFeature(QStringLiteral("dir_index"));
        addAvailableFeature(QStringLiteral("dir_nlink"));
        addAvailableFeature(QStringLiteral("ea_inode"));
        addAvailableFeature(QStringLiteral("encrypt"));
        addAvailableFeature(QStringLiteral("ext_attr"));
        addAvailableFeature(QStringLiteral("extent"));
        addAvailableFeature(QStringLiteral("extra_isize"));
        addAvailableFeature(QStringLiteral("filetype"));
        addAvailableFeature(QStringLiteral("flex_bg"));
        addAvailableFeature(QStringLiteral("has_journal"));
        addAvailableFeature(QStringLiteral("huge_file"));
        addAvailableFeature(QStringLiteral("inline_data"));
        addAvailableFeature(QStringLiteral("journal_dev"));
        addAvailableFeature(QStringLiteral("large_dir"));
        addAvailableFeature(QStringLiteral("large_file"));
        addAvailableFeature(QStringLiteral("metadata_csum"));
        addAvailableFeature(QStringLiteral("metadata_csum_seed"));
        addAvailableFeature(QStringLiteral("meta_bg"));
        addAvailableFeature(QStringLiteral("mmp"));
        addAvailableFeature(QStringLiteral("project"));
        addAvailableFeature(QStringLiteral("quota"));
        addAvailableFeature(QStringLiteral("resize_inode"));
        addAvailableFeature(QStringLiteral("sparse_super"));
        addAvailableFeature(QStringLiteral("sparse_super2"));
        addAvailableFeature(QStringLiteral("uninit_bg"));
        addAvailableFeature(QStringLiteral("lazy_itable_init=0"));
        addAvailableFeature(QStringLiteral("lazy_journal_init=0"));
    }
}

bool ext2::supportToolFound() const
{
    return
        m_GetUsed != cmdSupportNone &&
        m_GetLabel != cmdSupportNone &&
        m_SetLabel != cmdSupportNone &&
        m_Create != cmdSupportNone &&
        m_Check != cmdSupportNone &&
        m_UpdateUUID != cmdSupportNone &&
        m_Grow != cmdSupportNone &&
        m_Shrink != cmdSupportNone &&
        m_Copy != cmdSupportNone &&
        m_Move != cmdSupportNone &&
        m_Backup != cmdSupportNone &&
        m_GetUUID != cmdSupportNone;
}

FileSystem::SupportTool ext2::supportToolName() const
{
    return SupportTool(QStringLiteral("e2fsprogs"), QUrl(QStringLiteral("http://e2fsprogs.sf.net")));
}

qint64 ext2::maxCapacity() const
{
    return 16 * Capacity::unitFactor(Capacity::Unit::Byte, Capacity::Unit::TiB) - Capacity::unitFactor(Capacity::Unit::Byte, Capacity::Unit::MiB);
}

int ext2::maxLabelLength() const
{
    return 16;
}

qint64 ext2::readUsedCapacity(const QString& deviceNode) const
{
    ExternalCommand cmd(QStringLiteral("dumpe2fs"), { QStringLiteral("-h"), deviceNode });

    if (cmd.run()) {
        qint64 blockCount = -1;
        QRegularExpression re(QStringLiteral("Block count:\\s+(\\d+)"));
        QRegularExpressionMatch reBlockCount = re.match(cmd.output());

        if (reBlockCount.hasMatch())
            blockCount = reBlockCount.captured(1).toLongLong();

        qint64 freeBlocks = -1;
        re.setPattern(QStringLiteral("Free blocks:\\s+(\\d+)"));
        QRegularExpressionMatch reFreeBlocks = re.match(cmd.output());

        if (reFreeBlocks.hasMatch())
            freeBlocks = reFreeBlocks.captured(1).toLongLong();

        qint64 blockSize = -1;
        re.setPattern(QStringLiteral("Block size:\\s+(\\d+)"));
        QRegularExpressionMatch reBlockSize = re.match(cmd.output());

        if (reBlockSize.hasMatch())
            blockSize = reBlockSize.captured(1).toLongLong();

        if (blockCount > -1 && freeBlocks > -1 && blockSize > -1)
            return (blockCount - freeBlocks) * blockSize;
    }

    return -1;
}

static qint64 parseHumanSize(const QString& s)
{
    const QRegularExpressionMatch match = QRegularExpression(QStringLiteral("(\\d+)\\s*([kKmMgGtT])?")).match(s.trimmed());
    if (!match.hasMatch())
        return -1;

    qint64 value = match.captured(1).toLongLong();
    const QString unit = match.captured(2).toLower();
    if (unit == QStringLiteral("k"))
        value *= 1024;
    else if (unit == QStringLiteral("m"))
        value *= 1024LL * 1024;
    else if (unit == QStringLiteral("g"))
        value *= 1024LL * 1024 * 1024;
    else if (unit == QStringLiteral("t"))
        value *= 1024LL * 1024 * 1024 * 1024;

    return value;
}

void ext2::scan(const QString& deviceNode)
{
    clearProperties();

    if (m_GetUsed == cmdSupportNone)
        return;

    ExternalCommand cmd(QStringLiteral("dumpe2fs"), { QStringLiteral("-h"), deviceNode });
    if (!cmd.run() || cmd.exitCode() != 0)
        return;

    const QString output = cmd.output();
    QRegularExpression re;

    auto text = [&re, &output](const QString& pattern) -> QVariant {
        re.setPattern(pattern);
        const QRegularExpressionMatch match = re.match(output);
        return match.hasMatch() ? QVariant(match.captured(1).trimmed()) : QVariant();
    };
    auto number = [&text](const QString& pattern) -> QVariant {
        const QVariant value = text(pattern);
        return value.isValid() ? QVariant(value.toString().toLongLong()) : QVariant();
    };

    const QStringList features = text(QStringLiteral("Filesystem features:\\s+([^\\n]+)"))
                                    .toString().split(QLatin1Char(' '), Qt::SkipEmptyParts);
    const QVariant blockSize = number(QStringLiteral("Block size:\\s+(\\d+)"));
    const QVariant reservedBlocks = number(QStringLiteral("Reserved block count:\\s+(\\d+)"));
    const QVariant blockCount = number(QStringLiteral("Block count:\\s+(\\d+)"));

    addProperty(QStringLiteral("block-size"), blockSize, FileSystemProperty::DisplayType::Bytes, FileSystemProperty::Group::UnitSizes);
    if (features.contains(QStringLiteral("bigalloc")))
        addProperty(QStringLiteral("cluster-size"), number(QStringLiteral("Cluster size:\\s+(\\d+)")),
                    FileSystemProperty::DisplayType::Bytes, FileSystemProperty::Group::UnitSizes);

    addProperty(QStringLiteral("filesystem-features"), features.isEmpty() ? QVariant() : QVariant(features),
                FileSystemProperty::DisplayType::List, FileSystemProperty::Group::Capabilities);
    addProperty(QStringLiteral("format-version"), text(QStringLiteral("Filesystem revision #:\\s+([^\\n]+)")),
                FileSystemProperty::DisplayType::Text, FileSystemProperty::Group::Capabilities);

    addProperty(QStringLiteral("reserved-block-count"), reservedBlocks, FileSystemProperty::DisplayType::Number, FileSystemProperty::Group::Reserved);
    QVariant reservedSpace;
    if (reservedBlocks.isValid() && blockSize.isValid())
        reservedSpace = QVariant(reservedBlocks.toLongLong() * blockSize.toLongLong());
    addProperty(QStringLiteral("reserved-space"), reservedSpace, FileSystemProperty::DisplayType::Bytes, FileSystemProperty::Group::Reserved);
    if (reservedBlocks.isValid() && blockCount.isValid() && blockCount.toLongLong() > 0)
        addProperty(QStringLiteral("reserved-space-percent"), QVariant(100.0 * reservedBlocks.toLongLong() / blockCount.toLongLong()),
                    FileSystemProperty::DisplayType::Percent, FileSystemProperty::Group::Reserved);
    addProperty(QStringLiteral("reserved-uid"), number(QStringLiteral("Reserved blocks uid:\\s+(\\d+)")),
                FileSystemProperty::DisplayType::Number, FileSystemProperty::Group::Reserved);
    addProperty(QStringLiteral("reserved-gid"), number(QStringLiteral("Reserved blocks gid:\\s+(\\d+)")),
                FileSystemProperty::DisplayType::Number, FileSystemProperty::Group::Reserved);

    addProperty(QStringLiteral("inode-size"), number(QStringLiteral("Inode size:\\s+(\\d+)")),
                FileSystemProperty::DisplayType::Bytes, FileSystemProperty::Group::Metadata);
    addProperty(QStringLiteral("inode-count"), number(QStringLiteral("Inode count:\\s+(\\d+)")),
                FileSystemProperty::DisplayType::Number, FileSystemProperty::Group::Metadata);

    if (features.contains(QStringLiteral("has_journal"))) {
        QVariant journalSize;
        const QVariant journalBlocks = number(QStringLiteral("Total journal blocks:\\s+(\\d+)"));
        if (journalBlocks.isValid() && blockSize.isValid())
            journalSize = QVariant(journalBlocks.toLongLong() * blockSize.toLongLong());
        else {
            const qint64 bytes = parseHumanSize(text(QStringLiteral("(?:Total journal size|Journal size):\\s+(\\S+)")).toString());
            if (bytes >= 0)
                journalSize = QVariant(bytes);
        }
        addProperty(QStringLiteral("journal-size"), journalSize, FileSystemProperty::DisplayType::Bytes, FileSystemProperty::Group::Journal);
    }
}

bool ext2::check(Report& report, const QString& deviceNode) const
{
    ExternalCommand cmd(report, QStringLiteral("e2fsck"), { QStringLiteral("-f"), QStringLiteral("-y"), QStringLiteral("-v"), deviceNode });
    return cmd.run(-1) && (cmd.exitCode() == 0 || cmd.exitCode() == 1 || cmd.exitCode() == 2 || cmd.exitCode() == 256);
}

bool ext2::create(Report& report, const QString& deviceNode)
{
    QStringList args = QStringList();

    if (!this->features().isEmpty()) {
        QStringList feature_list = QStringList();
        for (const auto& k : this->features().keys()) {
            const auto& v = this->features().value(k);
            if (v.typeId() == QMetaType::Type::Bool) {
                if (v.toBool())
                    feature_list << k;
            else
                    feature_list << (QStringLiteral("^") +  k);
            } else {
                qWarning() << "Ignoring feature" << k << "of type" << v.typeId() << "; requires type QMetaType::Type:Bool.";
            }
        }
        args << QStringLiteral("-O") << feature_list.join(QStringLiteral(","));
    }
    args << QStringLiteral("-qF") << deviceNode;

    ExternalCommand cmd(report, QStringLiteral("mkfs.ext2"), args);
    return cmd.run(-1) && cmd.exitCode() == 0;
}

bool ext2::resize(Report& report, const QString& deviceNode, qint64 length) const
{
    const QString len = QString::number(length / 512) + QStringLiteral("s");

    ExternalCommand cmd(report, QStringLiteral("resize2fs"), { deviceNode, len });
    return cmd.run(-1) && cmd.exitCode() == 0;
}

bool ext2::writeLabel(Report& report, const QString& deviceNode, const QString& newLabel)
{
    ExternalCommand cmd(report, QStringLiteral("e2label"), { deviceNode, newLabel });
    return cmd.run(-1) && cmd.exitCode() == 0;
}

bool ext2::writeLabelOnline(Report& report, const QString& deviceNode, const QString& mountPoint, const QString& newLabel)
{
    Q_UNUSED(mountPoint)
    return writeLabel(report, deviceNode, newLabel);
}

bool ext2::updateUUID(Report& report, const QString& deviceNode) const
{
    ExternalCommand cmd(report, QStringLiteral("tune2fs"), { QStringLiteral("-U"), QStringLiteral("random"), deviceNode });
    return cmd.run(-1) && cmd.exitCode() == 0;
}
}
