/*************************************************************************
 *  Copyright (C) 2017 by Pali Rohár <pali.rohar@gmail.com>              *
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

#include "fs/udf.h"

#include "util/externalcommand.h"
#include "util/capacity.h"
#include "util/report.h"

#include <KLocalizedString>

#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QString>
#include <QStringList>

namespace FS
{
constexpr qint64 MIN_UDF_BLOCKS = 300;
constexpr qint64 MAX_UDF_BLOCKS = ((1ULL << 32) - 1);

FileSystem::CommandSupportType udf::m_GetUsed = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType udf::m_SetLabel = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType udf::m_UpdateUUID = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType udf::m_Create = FileSystem::cmdSupportNone;
bool udf::oldMkudffsVersion = false;

udf::udf(qint64 firstsector, qint64 lastsector, qint64 sectorsused, const QString& label) :
    FileSystem(firstsector, lastsector, sectorsused, label, FileSystem::Type::Udf)
{
}

void udf::init()
{
    m_GetUsed = findExternal(QStringLiteral("udfinfo"), {}, 1) ? cmdSupportFileSystem : cmdSupportNone;
    m_SetLabel = m_UpdateUUID = findExternal(QStringLiteral("udflabel"), {}, 1) ? cmdSupportFileSystem : cmdSupportNone;
    m_Create = findExternal(QStringLiteral("mkudffs"), {}, 1) ? cmdSupportFileSystem : cmdSupportNone;

    if (m_Create == cmdSupportFileSystem) {
        // Detect old mkudffs prior to version 1.1 by lack of --label option
        ExternalCommand cmd(QStringLiteral("mkudffs"), { QStringLiteral("--help") });
        oldMkudffsVersion = cmd.run(-1) && !cmd.output().contains(QStringLiteral("--label"));
    }
}

bool udf::supportToolFound() const
{
    return
        m_GetUsed != cmdSupportNone &&
        m_SetLabel != cmdSupportNone &&
        m_UpdateUUID != cmdSupportNone &&
        m_Create != cmdSupportNone;
}

FileSystem::SupportTool udf::supportToolName() const
{
    return SupportTool(QStringLiteral("udftools"), QUrl(QStringLiteral("https://github.com/pali/udftools")));
}

qint64 udf::minCapacity() const
{
    return MIN_UDF_BLOCKS * sectorSize();
}

qint64 udf::maxCapacity() const
{
    return MAX_UDF_BLOCKS * sectorSize();
}

int udf::maxLabelLength() const
{
    return 126;
}

QValidator* udf::labelValidator(QObject *parent) const
{
    QRegularExpressionValidator *m_LabelValidator = new QRegularExpressionValidator(parent);
    if (oldMkudffsVersion) {
        // Mkudffs from udftools prior to version 1.1 damages the label if it
        // contains non-ASCII characters.  Therefore do not allow a label with
        // such characters with old versions of mkudffs.
        m_LabelValidator->setRegularExpression(QRegularExpression(QStringLiteral("[\\x{0001}-\\x{007F}]{0,126}")));
    } else {
        // UDF label can only contain 126 bytes, either 126 ISO-8859-1
        // (Latin 1) characters or 63 UCS-2BE characters.
        m_LabelValidator->setRegularExpression(QRegularExpression(QStringLiteral("[\\x{0001}-\\x{00FF}]{0,126}|[\\x{0001}-\\x{FFFF}]{0,63}")));
    }
    return m_LabelValidator;
}

qint64 udf::readUsedCapacity(const QString& deviceNode) const
{
    ExternalCommand cmd(QStringLiteral("udfinfo"), { QStringLiteral("--utf8"), deviceNode });
    if (!cmd.run(-1) || cmd.exitCode() != 0)
        return -1;

    QRegularExpressionMatch reBlockSize = QRegularExpression(QStringLiteral("^blocksize=([0-9]+)$"), QRegularExpression::MultilineOption).match(cmd.output());
    QRegularExpressionMatch reUsedBlocks = QRegularExpression(QStringLiteral("^usedblocks=([0-9]+)$"), QRegularExpression::MultilineOption).match(cmd.output());

    if (!reBlockSize.hasMatch() || !reUsedBlocks.hasMatch())
        return -1;

    qint64 blockSize = reBlockSize.captured(1).toLongLong();
    qint64 usedBlocks = reUsedBlocks.captured(1).toLongLong();

    return usedBlocks * blockSize;
}

bool udf::writeLabel(Report& report, const QString& deviceNode, const QString& newLabel)
{
    ExternalCommand cmd(report, QStringLiteral("udflabel"), { QStringLiteral("--utf8"), deviceNode, newLabel });
    return cmd.run(-1) && cmd.exitCode() == 0;
}

bool udf::updateUUID(Report& report, const QString& deviceNode) const
{
    ExternalCommand cmd(report, QStringLiteral("udflabel"), { QStringLiteral("--utf8"), QStringLiteral("--uuid=random"), deviceNode });
    return cmd.run(-1) && cmd.exitCode() == 0;
}

bool udf::create(Report& report, const QString& deviceNode)
{
    return createWithLabel(report, deviceNode, QString());
}

bool udf::createWithLabel(Report& report, const QString& deviceNode, const QString& label)
{
    // It is not possible to create UDF filesystem without a label or with empty label
    // When --lvid or --vid option is not specified, mkudffs use sane default
    QStringList labelArgs;
    if (!label.isEmpty()) {
        // The Volume Identifier (--vid) can only contain 30 bytes, either 30
        // ISO-8859-1 (Latin 1) characters or 15 UCS-2BE characters.  Store the
        // most characters possible in the Volume Identifier.  Either up to 15
        // UCS-2BE characters when a character needing 16-bit encoding is found in
        // the first 15 characters, or up to 30 characters when a character
        // needing 16-bit encoding is found in the second 15 characters.
        const QRegularExpression nonLatin1RegExp = QRegularExpression(QStringLiteral("[^\\x{0000}-\\x{00FF}]"));
        QString shortLabel = label.left(30);
        int firstNonLatin1Pos = shortLabel.indexOf(nonLatin1RegExp);
        if (firstNonLatin1Pos != -1 && firstNonLatin1Pos < 15)
            shortLabel = shortLabel.left(15);
        else if (firstNonLatin1Pos != -1 && firstNonLatin1Pos < 30)
            shortLabel = shortLabel.left(firstNonLatin1Pos);

        // UDF Logical Volume Identifier (--lvid) represents the label, but blkid
        // (from util-linux) prior to version v2.26 reads the Volume Identifier
        // (--vid).  Therefore for compatibility reasons store the label in both
        // locations.
        labelArgs << QStringLiteral("--lvid=") + label;
        labelArgs << QStringLiteral("--vid=") + shortLabel;
    }

    QStringList cmdArgs;
    cmdArgs << QStringLiteral("--utf8");
    // TODO: Add GUI option for choosing different optical disks and UDF revision
    // For now format as UDF revision 2.01 for hard disk media type
    cmdArgs << QStringLiteral("--media-type=hd");
    cmdArgs << QStringLiteral("--udfrev=0x201");
    // mkudffs from udftools prior to 1.1 is not able to detect logical (sector) size
    // and UDF block size must match logical sector size of underlying media
    cmdArgs << QStringLiteral("--blocksize=") + QString::number(sectorSize());
    cmdArgs << labelArgs;
    cmdArgs << deviceNode;

    ExternalCommand cmd(report, QStringLiteral("mkudffs"), cmdArgs);
    return cmd.run(-1) && cmd.exitCode() == 0;
}

}
