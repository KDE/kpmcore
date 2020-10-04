/*
    SPDX-FileCopyrightText: 2008-2011 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2013-2019 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2015 Teo Mrnjavac <teo@kde.org>
    SPDX-FileCopyrightText: 2020 Arnaud Ferraris <arnaud.ferraris@collabora.com>
    SPDX-FileCopyrightText: 2020 Gaël PORTAY <gael.portay@collabora.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "fs/fat16.h"

#include "util/externalcommand.h"
#include "util/capacity.h"
#include "util/report.h"

#include <KLocalizedString>

#include <QString>
#include <QStringList>

#include <ctime>

namespace FS
{
fat16::fat16(qint64 firstsector, qint64 lastsector, qint64 sectorsused, const QString& label, const QVariantMap& features, FileSystem::Type type) :
    fat12(firstsector, lastsector, sectorsused, label, features, type)
{
}

void fat16::init()
{
    m_Create = m_GetUsed = m_Check = findExternal(QStringLiteral("mkfs.fat"), {}, 1) ? cmdSupportFileSystem : cmdSupportNone;
    m_GetLabel = cmdSupportCore;
    m_SetLabel = findExternal(QStringLiteral("fatlabel")) ? cmdSupportFileSystem : cmdSupportNone;
    m_Move = cmdSupportCore;
    m_Copy = cmdSupportCore;
    m_Backup = cmdSupportCore;
    m_UpdateUUID = cmdSupportCore;
    m_Grow = findExternal(QStringLiteral("fatresize")) ? cmdSupportFileSystem : cmdSupportNone;
    m_Shrink = findExternal(QStringLiteral("fatresize")) ? cmdSupportFileSystem : cmdSupportNone;
    m_GetUUID = cmdSupportCore;

    if (m_Create == cmdSupportFileSystem) {
        addAvailableFeature(QStringLiteral("sector-size"));
        addAvailableFeature(QStringLiteral("sectors-per-cluster"));
    }
}

bool fat16::supportToolFound() const
{
    return
        m_GetUsed != cmdSupportNone &&
        m_GetLabel != cmdSupportNone &&
        m_SetLabel != cmdSupportNone &&
        m_Create != cmdSupportNone &&
        m_Check != cmdSupportNone &&
        m_UpdateUUID != cmdSupportNone &&
//          m_Grow != cmdSupportNone &&
//          m_Shrink != cmdSupportNone &&
        m_Copy != cmdSupportNone &&
        m_Move != cmdSupportNone &&
        m_Backup != cmdSupportNone &&
        m_GetUUID != cmdSupportNone;
}

qint64 fat16::minCapacity() const
{
    return 16 * Capacity::unitFactor(Capacity::Unit::Byte, Capacity::Unit::MiB);
}

qint64 fat16::maxCapacity() const
{
    return 4 * Capacity::unitFactor(Capacity::Unit::Byte, Capacity::Unit::GiB) - Capacity::unitFactor(Capacity::Unit::Byte, Capacity::Unit::MiB);
}

bool fat16::create(Report& report, const QString& deviceNode)
{
    return createWithFatSize(report, deviceNode, 16);
}

bool fat16::resize(Report& report, const QString& deviceNode, qint64 length) const
{
    ExternalCommand cmd(report, QStringLiteral("fatresize"), { QStringLiteral("--verbose"), QStringLiteral("--size"), QString::number(length - 1), deviceNode });
    return cmd.run(-1) && cmd.exitCode() == 0;
}

}
