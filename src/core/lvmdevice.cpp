/*************************************************************************
 *  Copyright (C) 2008 by Volker Lanz <vl@fidra.de>                      *
 *  Copyright (C) 2016 by Andrius Štikonas <andrius@stikonas.eu>         *
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

#include "core/lvmdevice.h"

#include "core/partitiontable.h"
#include "util/externalcommand.h"

#include <QRegularExpression>

/** Constructs a representation of LVM device with functionning LV as Partition
    @param name Volume Group name
*/
LvmDevice::LvmDevice(const QString& name, const QString& iconname)
    : VolumeManagerDevice(name,
                          (QStringLiteral("/dev/") + name),
                          getPeSize(name),
                          getTotalPE(name),
                          iconname,
                          Device::LVM_Device)
    , m_peSize(getPeSize(name))
    , m_totalPE(getTotalPE(name))
    , m_allocPE(getAllocatedPE(name))
    , m_freePE(getFreePE(name))
{
    //called to initialize member variables
    //refresh();
    initPartitions();
}
/*
void LvmDevice::refresh() const
{
}
*/

void LvmDevice::initPartitions() const
{
    //PartitionTable* pTable = new PartitionTable(PartitionTable::TableType::vmd,0, )
}

qint32 LvmDevice::getPeSize(const QString& vgname)
{
    ExternalCommand cmd(QStringLiteral("lvm"),
            { QStringLiteral("vgdisplay"),
              QStringLiteral("--units"),
              QStringLiteral("B"),
              vgname});
    if (cmd.run(-1) && cmd.exitCode() == 0) {
        QRegularExpression re(QStringLiteral("PE Size\\h+(\\d+)"));
        QRegularExpressionMatch match = re.match(cmd.output());
        if (match.hasMatch()) {
            return match.captured(1).toInt();
        }
    }
    return -1;
}

qint32 LvmDevice::getTotalPE(const QString& vgname)
{
    ExternalCommand cmd(QStringLiteral("lvm"),
            { QStringLiteral("vgdisplay"),
              QStringLiteral("--units"),
              QStringLiteral("B"),
              vgname});
    if (cmd.run(-1) && cmd.exitCode() == 0) {
        QRegularExpression re(QStringLiteral("Total PE\\h+(\\d+)"));
        QRegularExpressionMatch match = re.match(cmd.output());
        if (match.hasMatch()) {
            return match.captured(1).toInt();
        }
    }
    return -1;
}

qint32 LvmDevice::getAllocatedPE(const QString& vgname)
{
    ExternalCommand cmd(QStringLiteral("lvm"),
            { QStringLiteral("vgdisplay"),
              QStringLiteral("--units"),
              QStringLiteral("B"),
              vgname});
    if (cmd.run(-1) && cmd.exitCode() == 0) {
        QRegularExpression re(QStringLiteral("Alloc PE.*\\h+(\\d+)"));
        QRegularExpressionMatch match = re.match(cmd.output());
        if (match.hasMatch()) {
            return match.captured(1).toInt();
        }
    }
    return -1;
}

qint32 LvmDevice::getFreePE(const QString& vgname)
{
    ExternalCommand cmd(QStringLiteral("lvm"),
            { QStringLiteral("vgdisplay"),
              QStringLiteral("--units"),
              QStringLiteral("B"),
              vgname});
    if (cmd.run(-1) && cmd.exitCode() == 0) {
        QRegularExpression re(QStringLiteral("Free  PE.*\\h+(\\d+)"));
        QRegularExpressionMatch match = re.match(cmd.output());
        if (match.hasMatch()) {
            return match.captured(1).toInt();
        }
    }
    return -1;
}
