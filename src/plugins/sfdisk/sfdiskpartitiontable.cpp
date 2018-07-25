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

#include "plugins/sfdisk/sfdiskpartitiontable.h"

#include "backend/corebackend.h"
#include "backend/corebackendmanager.h"

#include "core/partition.h"
#include "core/device.h"
#include "core/raid/softwareraid.h"

#include "fs/filesystem.h"

#include "util/report.h"
#include "util/externalcommand.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>

#include <KLocalizedString>

SfdiskPartitionTable::SfdiskPartitionTable(const Device* d) :
    CoreBackendPartitionTable(),
    m_device(d)
{
}

SfdiskPartitionTable::~SfdiskPartitionTable()
{
}

bool SfdiskPartitionTable::open()
{
    return true;
}

bool SfdiskPartitionTable::commit(quint32 timeout)
{
    if (m_device->type() == Device::Type::SoftwareRAID_Device)
        ExternalCommand(QStringLiteral("udevadm"), { QStringLiteral("control"), QStringLiteral("--stop-exec-queue") }).run();

    ExternalCommand(QStringLiteral("udevadm"), { QStringLiteral("settle"), QStringLiteral("--timeout=") + QString::number(timeout) }).run();
    ExternalCommand(QStringLiteral("blockdev"), { QStringLiteral("--rereadpt"), m_device->deviceNode() }).run();
    ExternalCommand(QStringLiteral("udevadm"), { QStringLiteral("trigger") }).run();

    if (m_device->type() == Device::Type::SoftwareRAID_Device)
        ExternalCommand(QStringLiteral("udevadm"), { QStringLiteral("control"), QStringLiteral("--start-exec-queue") }).run();

    return true;
}

QString SfdiskPartitionTable::createPartition(Report& report, const Partition& partition)
{
    if ( !(partition.roles().has(PartitionRole::Extended) || partition.roles().has(PartitionRole::Logical) || partition.roles().has(PartitionRole::Primary) ) ) {
        report.line() << xi18nc("@info:progress", "Unknown partition role for new partition <filename>%1</filename> (roles: %2)", partition.deviceNode(), partition.roles().toString());
        return QString();
    }

    QByteArray type = QByteArray();
    if (partition.roles().has(PartitionRole::Extended))
        type = QByteArrayLiteral(" type=5");

    // NOTE: at least on GPT partition types "are" partition flags
    ExternalCommand createCommand(report, QStringLiteral("sfdisk"), { QStringLiteral("--force"), QStringLiteral("--append"), partition.devicePath() } );
    if ( createCommand.write(QByteArrayLiteral("start=") + QByteArray::number(partition.firstSector()) +
                                                        type +
                                                        QByteArrayLiteral(" size=") + QByteArray::number(partition.length()) + QByteArrayLiteral("\nwrite\n")) && createCommand.start(-1) ) {
        QRegularExpression re(QStringLiteral("Created a new partition (\\d+)"));
        QRegularExpressionMatch rem = re.match(createCommand.output());

        if (rem.hasMatch()) {
            if ( partition.devicePath().back().isDigit() )
                return partition.devicePath() + QLatin1Char('p') + rem.captured(1);
            else
                return partition.devicePath() + rem.captured(1);
        }
    }

    report.line() << xi18nc("@info:progress", "Failed to add partition <filename>%1</filename> to device <filename>%2</filename>.", partition.deviceNode(), m_device->deviceNode());

    return QString();
}

bool SfdiskPartitionTable::deletePartition(Report& report, const Partition& partition)
{
    ExternalCommand deleteCommand(report, QStringLiteral("sfdisk"), { QStringLiteral("--force"), QStringLiteral("--delete"), partition.devicePath(), QString::number(partition.number()) } );
    if (deleteCommand.run(-1) && deleteCommand.exitCode() == 0)
        return true;

    report.line() << xi18nc("@info:progress", "Could not delete partition <filename>%1</filename>.", partition.devicePath());
    return false;
}

bool SfdiskPartitionTable::updateGeometry(Report& report, const Partition& partition, qint64 sectorStart, qint64 sectorEnd)
{
    ExternalCommand sfdiskCommand(report, QStringLiteral("sfdisk"), { QStringLiteral("--force"), partition.devicePath(), QStringLiteral("-N"), QString::number(partition.number()) } );
    if ( sfdiskCommand.write(QByteArrayLiteral("start=") + QByteArray::number(sectorStart) +
                                                        QByteArrayLiteral(" size=") + QByteArray::number(sectorEnd - sectorStart + 1) +
                                                        QByteArrayLiteral("\nY\n"))
                                                        && sfdiskCommand.start(-1) && sfdiskCommand.exitCode() == 0) {
        return true;
    }

    report.line() << xi18nc("@info:progress", "Could not set geometry for partition <filename>%1</filename> while trying to resize/move it.", partition.devicePath());
    return false;
}

bool SfdiskPartitionTable::clobberFileSystem(Report& report, const Partition& partition)
{
    ExternalCommand wipeCommand(report, QStringLiteral("wipefs"), { QStringLiteral("--all"), partition.partitionPath() } );
    if (wipeCommand.run(-1) && wipeCommand.exitCode() == 0)
        return true;

    report.line() << xi18nc("@info:progress", "Failed to erase filesystem signature on partition <filename>%1</filename>.", partition.partitionPath());

    return false;
}

bool SfdiskPartitionTable::resizeFileSystem(Report& report, const Partition& partition, qint64 newLength)
{
    // sfdisk does not have any partition resize capabilities
    Q_UNUSED(report)
    Q_UNUSED(partition)
    Q_UNUSED(newLength)

    return false;
}

FileSystem::Type SfdiskPartitionTable::detectFileSystemBySector(Report& report, const Device& device, qint64 sector)
{
    FileSystem::Type type = FileSystem::Type::Unknown;

    ExternalCommand jsonCommand(QStringLiteral("sfdisk"), { QStringLiteral("--json"), device.deviceNode() } );
    if (jsonCommand.run(-1) && jsonCommand.exitCode() == 0) {
        const QJsonArray partitionTable = QJsonDocument::fromJson(jsonCommand.rawOutput()).object()[QLatin1String("partitiontable")].toObject()[QLatin1String("partitions")].toArray();
        for (const auto &partition : partitionTable) {
            const QJsonObject partitionObject = partition.toObject();
            const qint64 start = partitionObject[QLatin1String("start")].toVariant().toLongLong();
            if (start == sector) {
                const QString deviceNode = partitionObject[QLatin1String("node")].toString();
                type = CoreBackendManager::self()->backend()->detectFileSystem(deviceNode);
                return type;
            }
        }
    }

    report.line() << xi18nc("@info:progress", "Could not determine file system of partition at sector %1 on device <filename>%2</filename>.", sector, device.deviceNode());

    return type;
}

static struct {
    FileSystem::Type type;
    QLatin1String partitionType[2]; // GPT, MBR
} typemap[] = {
    { FileSystem::Type::Btrfs, { QLatin1String("0FC63DAF-8483-4772-8E79-3D69D8477DE4"), QLatin1String("83") } },
    { FileSystem::Type::Ext2, { QLatin1String("0FC63DAF-8483-4772-8E79-3D69D8477DE4"), QLatin1String("83") } },
    { FileSystem::Type::Ext3, { QLatin1String("0FC63DAF-8483-4772-8E79-3D69D8477DE4"), QLatin1String("83") } },
    { FileSystem::Type::Ext4, { QLatin1String("0FC63DAF-8483-4772-8E79-3D69D8477DE4"), QLatin1String("83") } },
    { FileSystem::Type::LinuxSwap, { QLatin1String("0657FD6D-A4AB-43C4-84E5-0933C84B4F4F"), QLatin1String("82") } },
    { FileSystem::Type::Fat12, { QLatin1String("EBD0A0A2-B9E5-4433-87C0-68B6B72699C7"), QLatin1String("6") } },
    { FileSystem::Type::Fat16, { QLatin1String("EBD0A0A2-B9E5-4433-87C0-68B6B72699C7"), QLatin1String("6") } },
    { FileSystem::Type::Fat32, { QLatin1String("EBD0A0A2-B9E5-4433-87C0-68B6B72699C7"), QLatin1String("7") } },
    { FileSystem::Type::Nilfs2, { QLatin1String("0FC63DAF-8483-4772-8E79-3D69D8477DE4"), QLatin1String("83") } },
    { FileSystem::Type::Ntfs, { QLatin1String("EBD0A0A2-B9E5-4433-87C0-68B6B72699C7"), QLatin1String("7") } },
    { FileSystem::Type::Exfat, { QLatin1String("EBD0A0A2-B9E5-4433-87C0-68B6B72699C7"), QLatin1String("7") } },
    { FileSystem::Type::ReiserFS, { QLatin1String("0FC63DAF-8483-4772-8E79-3D69D8477DE4"), QLatin1String("83") } },
    { FileSystem::Type::Reiser4, { QLatin1String("0FC63DAF-8483-4772-8E79-3D69D8477DE4"), QLatin1String("83") } },
    { FileSystem::Type::Xfs, { QLatin1String("0FC63DAF-8483-4772-8E79-3D69D8477DE4"), QLatin1String("83") } },
    { FileSystem::Type::Jfs, { QLatin1String("0FC63DAF-8483-4772-8E79-3D69D8477DE4"), QLatin1String("83") } },
    { FileSystem::Type::Hfs, { QLatin1String("48465300-0000-11AA-AA11-00306543ECAC"), QLatin1String("af")} },
    { FileSystem::Type::HfsPlus, { QLatin1String("48465300-0000-11AA-AA11-00306543ECAC"), QLatin1String("af") } },
    { FileSystem::Type::Udf, { QLatin1String("EBD0A0A2-B9E5-4433-87C0-68B6B72699C7"), QLatin1String("7") } }
    // Add ZFS too
};

static QLatin1String getPartitionType(FileSystem::Type t, PartitionTable::TableType tableType)
{
    quint8 type;
    switch (tableType) {
    case PartitionTable::gpt:
        type = 0;
        break;
    case PartitionTable::msdos:
    case PartitionTable::msdos_sectorbased:
        type = 1;
        break;
    default:;
        return QLatin1String();
    }
    for (quint32 i = 0; i < sizeof(typemap) / sizeof(typemap[0]); i++)
        if (typemap[i].type == t)
            return typemap[i].partitionType[type];

    return QLatin1String();
}

bool SfdiskPartitionTable::setPartitionSystemType(Report& report, const Partition& partition)
{
    QString partitionType = getPartitionType(partition.fileSystem().type(), m_device->partitionTable()->type());
    if (partitionType.isEmpty())
        return true;
    ExternalCommand sfdiskCommand(report, QStringLiteral("sfdisk"), { QStringLiteral("--part-type"), m_device->deviceNode(), QString::number(partition.number()),
                partitionType } );
    return sfdiskCommand.run(-1) && sfdiskCommand.exitCode() == 0;
}

bool SfdiskPartitionTable::setFlag(Report& report, const Partition& partition, PartitionTable::Flag flag, bool state)
{
    // We only allow setting one active partition per device
    if (flag == PartitionTable::FlagBoot && state == true) {
        ExternalCommand sfdiskCommand(report, QStringLiteral("sfdisk"), { QStringLiteral("--activate"), m_device->deviceNode(), QString::number(partition.number()) } );
        if (sfdiskCommand.run(-1) && sfdiskCommand.exitCode() == 0)
            return true;
        else
            return false;
    } else if (flag == PartitionTable::FlagBoot && state == false) {
        ExternalCommand sfdiskCommand(report, QStringLiteral("sfdisk"), { QStringLiteral("--activate"), m_device->deviceNode(), QStringLiteral("-") } );
        if (sfdiskCommand.run(-1) && sfdiskCommand.exitCode() == 0)
            return true;
        // FIXME: Do not return false since we have no way of checking if partition table is MBR
    }

    if (flag == PartitionTable::FlagEsp && state == true) {
        ExternalCommand sfdiskCommand(report, QStringLiteral("sfdisk"), { QStringLiteral("--part-type"), m_device->deviceNode(), QString::number(partition.number()),
                QStringLiteral("C12A7328-F81F-11D2-BA4B-00A0C93EC93B") } );
        if (sfdiskCommand.run(-1) && sfdiskCommand.exitCode() == 0)
            return true;
        else
            return false;
    }
    if (flag == PartitionTable::FlagEsp && state == false)
        setPartitionSystemType(report, partition);

    if (flag == PartitionTable::FlagBiosGrub && state == true) {
        ExternalCommand sfdiskCommand(report, QStringLiteral("sfdisk"), { QStringLiteral("--part-type"), m_device->deviceNode(), QString::number(partition.number()),
                QStringLiteral("21686148-6449-6E6F-744E-656564454649") } );
        if (sfdiskCommand.run(-1) && sfdiskCommand.exitCode() == 0)
            return true;
        else
            return false;
    }
    if (flag == PartitionTable::FlagBiosGrub && state == false)
        setPartitionSystemType(report, partition);

    return true;
}
