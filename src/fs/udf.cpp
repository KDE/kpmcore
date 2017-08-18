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

udf::udf(qint64 firstsector, qint64 lastsector, qint64 sectorsused, const QString& label) :
    FileSystem(firstsector, lastsector, sectorsused, label, FileSystem::Udf)
{
}

void udf::init()
{
    m_Create = findExternal(QStringLiteral("mkudffs"), {}, 1) ? cmdSupportFileSystem : cmdSupportNone;
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
    int blkSize = blockSize(report, deviceNode);
    if (blkSize == -1)
        return false;

    ExternalCommand cmd(report, QStringLiteral("mkudffs"), {
        QStringLiteral("--utf8"),
        // TODO: Add GUI option for choosing different optical disks and UDF revision
        // For now format as UDF revision 2.01 for hard disk media type
        QStringLiteral("--media-type=hd"),
        QStringLiteral("--udfrev=0x201"),
        QStringLiteral("--blocksize=") + QString::number(blkSize),
        // TODO: Pass label as udf::create() parameter
        // QStringLiteral("--lvid=") + label,
        // QStringLiteral("--vid=") + shortlabel,
        deviceNode
    });
    return cmd.run(-1) && cmd.exitCode() == 0;
}

bool udf::createWithLabel(Report& report, const QString& deviceNode, const QString& label)
{
    int blkSize = blockSize(report, deviceNode);
    if (blkSize == -1)
        return false;

    ExternalCommand cmd(report, QStringLiteral("mkudffs"), {
        QStringLiteral("--utf8"),
        // TODO: Add GUI option for choosing different optical disks and UDF revision
        // For now format as UDF revision 2.01 for hard disk media type
        QStringLiteral("--media-type=hd"),
        QStringLiteral("--udfrev=0x201"),
        QStringLiteral("--blocksize=") + QString::number(blkSize),
        // TODO: Pass label as udf::create() parameter
        QStringLiteral("--lvid=") + label,
        // QStringLiteral("--vid=") + shortlabel,
        deviceNode
    });
    return cmd.run(-1) && cmd.exitCode() == 0;
}

int udf::blockSize(Report& report, const QString& deviceNode)
{
    // mkudffs from udftools prior to 1.1 does not check for partition limits and crashes
    if ( length() > MAX_UDF_BLOCKS ) {
        report.line() << xi18nc("@info:status", "Partition is too large");
        return -1;
    }
    if ( length() < MIN_UDF_BLOCKS ) {
        report.line() << xi18nc("@info:status", "Partition is too small");
        return -1;
    }

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
