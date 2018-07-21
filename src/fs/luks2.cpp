/*************************************************************************
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

#include "fs/luks2.h"

#include "util/externalcommand.h"
#include "util/report.h"

#include <QRegularExpression>

#include <KLocalizedString>

namespace FS
{

luks2::luks2(qint64 firstsector, qint64 lastsector, qint64 sectorsused, const QString& label)
    : luks(firstsector, lastsector, sectorsused, label, FileSystem::Type::Luks2)
{
}

luks2::~luks2()
{
}

FileSystem::Type luks2::type() const
{
    if (m_isCryptOpen && m_innerFs)
        return m_innerFs->type();
    return FileSystem::Type::Luks2;
}

bool luks2::create(Report& report, const QString& deviceNode)
{
    Q_ASSERT(m_innerFs);
    Q_ASSERT(!m_passphrase.isEmpty());

    ExternalCommand createCmd(report, QStringLiteral("cryptsetup"),
                              { QStringLiteral("-s"),
                                QStringLiteral("512"),
                                QStringLiteral("--batch-mode"),
                                QStringLiteral("--force-password"),
                                QStringLiteral("--type"), QStringLiteral("luks2"),
                                QStringLiteral("luksFormat"),
                                deviceNode });
    if (!( createCmd.write(m_passphrase.toLocal8Bit() + '\n') &&
                createCmd.start(-1) && createCmd.exitCode() == 0))
    {
        return false;
    }

    ExternalCommand openCmd(report, QStringLiteral("cryptsetup"),
                              { QStringLiteral("open"),
                                deviceNode,
                                suggestedMapperName(deviceNode) });

    if (!( openCmd.write(m_passphrase.toLocal8Bit() + '\n') && openCmd.start(-1)))
        return false;

    setPayloadSize();
    scan(deviceNode);

    if (mapperName().isEmpty())
        return false;

    if (!m_innerFs->create(report, mapperName()))
        return false;

    return true;
}

bool luks2::resize(Report& report, const QString& deviceNode, qint64 newLength) const
{
    Q_ASSERT(m_innerFs);

    if (mapperName().isEmpty())
        return false;

    if ( newLength - length() * sectorSize() > 0 )
    {
        ExternalCommand cryptResizeCmd(report, QStringLiteral("cryptsetup"), { QStringLiteral("resize"), mapperName() });
        report.line() << xi18nc("@info:progress", "Resizing LUKS crypt on partition <filename>%1</filename>.", deviceNode);

        if (m_KeyLocation == KeyLocation::keyring) {
            if (m_passphrase.isEmpty())
                return false;
            cryptResizeCmd.write(m_passphrase.toLocal8Bit() + '\n');
        }
        if (!cryptResizeCmd.start(-1))
            return false;
        if ( cryptResizeCmd.exitCode() == 0 )
            return m_innerFs->resize(report, mapperName(), m_PayloadSize);
    }
    else if (m_innerFs->resize(report, mapperName(), m_PayloadSize))
    {
        ExternalCommand cryptResizeCmd(report, QStringLiteral("cryptsetup"),
                {  QStringLiteral("--size"), QString::number(m_PayloadSize / 512), // FIXME, LUKS2 can have different sector sizes
                   QStringLiteral("resize"), mapperName() });
        report.line() << xi18nc("@info:progress", "Resizing LUKS crypt on partition <filename>%1</filename>.", deviceNode);
        if (m_KeyLocation == KeyLocation::keyring) {
            if (m_passphrase.isEmpty())
                return false;
            cryptResizeCmd.write(m_passphrase.toLocal8Bit() + '\n');
        }
        if (!cryptResizeCmd.start(-1))
            return false;
        if ( cryptResizeCmd.exitCode() == 0 )
            return true;
    }
    report.line() << xi18nc("@info:progress", "Resizing encrypted file system on partition <filename>%1</filename> failed.", deviceNode);
    return false;
}

luks::KeyLocation luks2::keyLocation()
{
    m_KeyLocation = KeyLocation::unknown;
    ExternalCommand statusCmd(QStringLiteral("cryptsetup"), { QStringLiteral("status"), mapperName() });
    if (statusCmd.run(-1) && statusCmd.exitCode() == 0) {
        QRegularExpression re(QStringLiteral("key location:\\s+(\\w+)"));
        QRegularExpressionMatch rem = re.match(statusCmd.output());
        if (rem.hasMatch()) {
            if (rem.captured(1) == QStringLiteral("keyring"))
                m_KeyLocation = KeyLocation::keyring;
            else if (rem.captured(1) == QStringLiteral("dm-crypt"))
                m_KeyLocation = KeyLocation::dmcrypt;
        }
    }

    return m_KeyLocation;
}

}
