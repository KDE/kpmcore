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
#include <QString>
#include <QStringList>

#include <fcntl.h>
#include <linux/fs.h>
#include <sys/ioctl.h>
#include <unistd.h>

namespace FS
{
constexpr qint64 MIN_UDF_BLOCKS = 282;
constexpr qint64 MAX_UDF_BLOCKS = ((1ULL << 32) - 1);

FileSystem::CommandSupportType udf::m_Create = FileSystem::cmdSupportNone;
bool udf::m_OnlyAsciiLabel = false;

udf::udf(qint64 firstsector, qint64 lastsector, qint64 sectorsused, const QString& label) :
    FileSystem(firstsector, lastsector, sectorsused, label, FileSystem::Udf)
{
}

void udf::init()
{
    m_Create = findExternal(QStringLiteral("mkudffs"), {}, 1) ? cmdSupportFileSystem : cmdSupportNone;

    // Detect old mkudffs prior to version 1.1 by lack of --label option
    ExternalCommand cmd(QStringLiteral("mkudffs"), { QStringLiteral("--help") });
    m_OnlyAsciiLabel = cmd.run(-1) && !cmd.output().contains(QStringLiteral("--label"));
}

bool udf::supportToolFound() const
{
    return m_Create != cmdSupportNone;
}

FileSystem::SupportTool udf::supportToolName() const
{
    return SupportTool(QStringLiteral("udftools"), QUrl(QStringLiteral("https://github.com/pali/udftools")));
}


qint64 udf::minCapacity() const
{
    // TODO: Capacity depends on logical (sector) size of disk, now hardcoded as 512
    return MIN_UDF_BLOCKS * 512;
}

qint64 udf::maxCapacity() const
{
    // TODO: Capacity depends on logical (sector) size of disk, now hardcoded as 4096
    return MAX_UDF_BLOCKS * 4096;
}

qint64 udf::maxLabelLength() const
{
    return 126; // and only 63 if label contains character above U+FF
}

bool udf::create(Report& report, const QString& deviceNode)
{
    return createWithLabel(report, deviceNode, QString());
}

bool udf::createWithLabel(Report& report, const QString& deviceNode, const QString& label)
{
    // mkudffs from udftools prior to 1.1 does not check for partition limits and crashes
    if (length() > MAX_UDF_BLOCKS) {
        report.line() << xi18nc("@info:status", "Partition is too large");
        return false;
    }
    if (length() < MIN_UDF_BLOCKS) {
        report.line() << xi18nc("@info:status", "Partition is too small");
        return false;
    }

    int blkSize = blockSize(report, deviceNode);
    if (blkSize == -1)
        return false;

    // It is not possible to create UDF filesystem without a label or with empty label
    // When --lvid or --vid option is not specified, mkudffs use sane default
    QStringList labelArgs;
    if (!label.isEmpty()) {
        const QRegularExpression nonAsciiRegExp = QRegularExpression(QStringLiteral("[^\\x{0000}-\\x{007F}]"));
        const QRegularExpression nonLatin1RegExp = QRegularExpression(QStringLiteral("[^\\x{0000}-\\x{00FF}]"));

        // Mkudffs from udftools prior to version 1.1 damages the label if it
        // contains non-ASCII characters.  Therefore do not allow a label with
        // such characters with old versions of mkudffs.
        if (m_OnlyAsciiLabel && label.contains(nonAsciiRegExp)) {
            report.line() << xi18nc("@info:status", "mkudffs prior to version 1.1 does not support non-ASCII characters in the label");
            return false;
        }

        // NOTE: According to the OSTA specification, UDF supports only strings
        // encoded in 8-bit or 16-bit OSTA Compressed Unicode format.  They are
        // equivalent to ISO-8859-1 (Latin 1) and UCS-2BE respectively.
        // Conversion from UTF-8 passed on the command line to OSTA format is done
        // by mkudffs.  Strictly speaking UDF does not support UTF-16 as the UDF
        // specification was created before the introduction of UTF-16, but lots
        // of UDF tools are able to decode UTF-16 including UTF-16 Surrogate pairs
        // outside the BMP (Basic Multilingual Plane).
        //
        // The Volume Identifier (--vid) can only contain 30 bytes, either 30
        // ISO-8859-1 (Latin 1) characters or 15 UCS-2BE characters.  Store the
        // most characters possible in the Volume Identifier.  Either up to 15
        // UCS-2BE characters when a character needing 16-bit encoding is found in
        // the first 15 characters, or up to 30 characters when a character
        // needing 16-bit encoding is found in the second 15 characters.
        QString shortLabel = label.left(30);
        int firstNonLatin1Pos = shortLabel.indexOf(nonLatin1RegExp);
        if (firstNonLatin1Pos != -1 && firstNonLatin1Pos < 15)
            shortLabel = shortLabel.left(15);
        else if (firstNonLatin1Pos != -1 && firstNonLatin1Pos < 30)
            shortLabel = shortLabel.left(firstNonLatin1Pos);

        labelArgs << QStringLiteral("--lvid=") + label;
        labelArgs << QStringLiteral("--vid=") + shortLabel;
    }

    QStringList cmdArgs;
    cmdArgs << QStringLiteral("--utf8");
    // TODO: Add GUI option for choosing different optical disks and UDF revision
    // For now format as UDF revision 2.01 for hard disk media type
    cmdArgs << QStringLiteral("--media-type=hd");
    cmdArgs << QStringLiteral("--udfrev=0x201");
    cmdArgs << QStringLiteral("--blocksize=") + QString::number(blkSize);
    cmdArgs << labelArgs;
    cmdArgs << deviceNode;

    ExternalCommand cmd(report, QStringLiteral("mkudffs"), cmdArgs);
    return cmd.run(-1) && cmd.exitCode() == 0;
}

int udf::blockSize(Report& report, const QString& deviceNode)
{
    // mkudffs from udftools prior to 1.1 is not able to detect logical (sector) size
    // and UDF block size must match logical sector size of underlying media
    int fd = open(deviceNode.toLocal8Bit().data(), O_RDONLY);
    if ( fd < 0 ) {
        report.line() << xi18nc("@info:status", "Cannot open device node");
        return -1;
    }
    int blksize;
    int ret = ioctl(fd, BLKSSZGET, &blksize);
    close(fd);

    if ( ret != 0 ) {
        report.line() << xi18nc("@info:status", "Cannot read logical (sector) size of device node");
        return -1;
    }
    return blksize;
}

}
