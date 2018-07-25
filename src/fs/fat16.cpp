/*************************************************************************
 *  Copyright (C) 2008,2009,2011 by Volker Lanz <vl@fidra.de>            *
 *  Copyright (C) 2016-2017 by Andrius Štikonas <andrius@stikonas.eu>    *
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

#include "fs/fat16.h"

#include "util/externalcommand.h"
#include "util/capacity.h"
#include "util/report.h"

#include <KLocalizedString>

#include <QRegularExpression>
#include <QString>
#include <QStringList>

#include <ctime>

namespace FS
{
fat16::fat16(qint64 firstsector, qint64 lastsector, qint64 sectorsused, const QString& label) :
    fat12(firstsector, lastsector, sectorsused, label, FileSystem::Type::Fat16)
{
}

fat16::fat16(qint64 firstsector, qint64 lastsector, qint64 sectorsused, const QString& label, FileSystem::Type type) :
    fat12(firstsector, lastsector, sectorsused, label, type)
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
    m_UpdateUUID = findExternal(QStringLiteral("dd")) ? cmdSupportFileSystem : cmdSupportNone;
    m_Grow = findExternal(QStringLiteral("fatresize")) ? cmdSupportFileSystem : cmdSupportNone;
    m_Shrink = findExternal(QStringLiteral("fatresize")) ? cmdSupportFileSystem : cmdSupportNone;
    m_GetUUID = cmdSupportCore;
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
    ExternalCommand cmd(report, QStringLiteral("mkfs.fat"), { QStringLiteral("-F16"), QStringLiteral("-I"), QStringLiteral("-v"), deviceNode });
    return cmd.run(-1) && cmd.exitCode() == 0;
}

bool fat16::resize(Report& report, const QString& deviceNode, qint64 length) const
{
    ExternalCommand cmd(report, QStringLiteral("fatresize"), { QStringLiteral("--verbose"), QStringLiteral("--size"), QString::number(length), deviceNode });
    return cmd.run(-1) && cmd.exitCode() == 0;
}

}
