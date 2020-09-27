/*
    SPDX-FileCopyrightText: 2017-2019 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2020 Arnaud Ferraris <arnaud.ferraris@collabora.com>
    SPDX-FileCopyrightText: 2020 Gaël PORTAY <gael.portay@collabora.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "fs/luks2.h"

#include "util/externalcommand.h"
#include "util/report.h"

#include <QRegularExpression>

#include <KLocalizedString>

namespace FS
{

luks2::luks2(qint64 firstsector, qint64 lastsector, qint64 sectorsused, const QString& label, const QVariantMap& features)
    : luks(firstsector, lastsector, sectorsused, label, features, FileSystem::Type::Luks2)
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

    const qint64 sizeDiff = newLength - length() * sectorSize();
    const qint64 newPayloadSize = m_PayloadSize + sizeDiff;
    if ( sizeDiff > 0 ) // grow
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
            return m_innerFs->resize(report, mapperName(), newPayloadSize);
    }
    else if (m_innerFs->resize(report, mapperName(), newPayloadSize))
    {
        ExternalCommand cryptResizeCmd(report, QStringLiteral("cryptsetup"),
                {  QStringLiteral("--size"), QString::number(newPayloadSize / 512), // FIXME, LUKS2 can have different sector sizes
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
