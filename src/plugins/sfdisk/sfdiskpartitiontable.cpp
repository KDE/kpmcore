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

#include "fs/filesystem.h"

#include "util/report.h"
#include "util/externalcommand.h"

#include <unistd.h>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>

#include <KLocalizedString>

SfdiskPartitionTable::SfdiskPartitionTable(const QString& deviceNode) :
    CoreBackendPartitionTable(),
    m_deviceNode(deviceNode)
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
    if (!ExternalCommand(QStringLiteral("udevadm"), QStringList() << QStringLiteral("settle") << QStringLiteral("--timeout=") + QString::number(timeout)).run() &&
            !ExternalCommand(QStringLiteral("udevsettle"), QStringList() << QStringLiteral("--timeout=") + QString::number(timeout)).run())
        sleep(timeout);

    return true;
}

QString SfdiskPartitionTable::createPartition(Report& report, const Partition& partition)
{
    if ( !(partition.roles().has(PartitionRole::Extended) || partition.roles().has(PartitionRole::Logical) || partition.roles().has(PartitionRole::Primary) ) ) {
        report.line() << xi18nc("@info:progress", "Unknown partition role for new partition <filename>%1</filename> (roles: %2)", partition.deviceNode(), partition.roles().toString());
        return QString();
    }

    QByteArray type = QByteArray(); // FIXME add map between fs types and default partition types
    if (partition.roles().has(PartitionRole::Extended))
        type = QByteArrayLiteral(" type=5");

    // NOTE: at least on GPT partition types "are" partition flags
    ExternalCommand createCommand(report, QStringLiteral("sfdisk"), { QStringLiteral("--force"), QStringLiteral("--append"), partition.devicePath() } );
    if ( createCommand.start(-1) && createCommand.write(QByteArrayLiteral("start=") + QByteArray::number(partition.firstSector()) +
                                                        type +
                                                        QByteArrayLiteral(" size=") + QByteArray::number(partition.length()) + QByteArrayLiteral("\nwrite\n")) && createCommand.waitFor() ) {
        QRegularExpression re(QStringLiteral("Created a new partition (\\d)"));
        QRegularExpressionMatch rem = re.match(createCommand.output());
        if (rem.hasMatch())
            return partition.devicePath() + rem.captured(1);
    }

    report.line() << xi18nc("@info:progress", "Failed to add partition <filename>%1</filename> to device <filename>%2</filename>.", partition.deviceNode(), m_deviceNode);

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
    if ( sfdiskCommand.start(-1) && sfdiskCommand.write(QByteArrayLiteral("start=") + QByteArray::number(sectorStart) +
                                                        QByteArrayLiteral(" size=") + QByteArray::number(sectorEnd - sectorStart + 1) +
                                                        QByteArrayLiteral("\nY\n"))
                                                        && sfdiskCommand.waitFor() && sfdiskCommand.exitCode() == 0) {
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
    FileSystem::Type type = FileSystem::Unknown;

    ExternalCommand jsonCommand(QStringLiteral("sfdisk"), { QStringLiteral("--json"), device.deviceNode() } );
    if (jsonCommand.run(-1) && jsonCommand.exitCode() == 0) {
        const QJsonArray partitionTable = QJsonDocument::fromJson(jsonCommand.output().toLocal8Bit()).object()[QLatin1String("partitiontable")].toObject()[QLatin1String("partitions")].toArray();
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

bool SfdiskPartitionTable::setPartitionSystemType(Report& report, const Partition& partition)
{
    Q_UNUSED(report)
    Q_UNUSED(partition)

    return true;
}

bool SfdiskPartitionTable::setFlag(Report& report, const Partition& partition, PartitionTable::Flag flag, bool state)
{
    if (flag == PartitionTable::FlagBoot && state == true) {
        ExternalCommand sfdiskCommand(report, QStringLiteral("sfdisk"), { QStringLiteral("--activate"), m_deviceNode, QString::number(partition.number()) } );
        if (sfdiskCommand.run(-1) && sfdiskCommand.exitCode() == 0)
            return true;
        else
            return false;
    } else if (flag == PartitionTable::FlagBoot && state == false) {
        ExternalCommand sfdiskCommand(report, QStringLiteral("sfdisk"), { QStringLiteral("--activate"), m_deviceNode, QStringLiteral("-") } );
        if (sfdiskCommand.run(-1) && sfdiskCommand.exitCode() == 0)
            return true;
        // FIXME: Do not return false since we have no way of checking if partition table is MBR
    }

    if (flag == PartitionTable::FlagEsp && state == true) {
        ExternalCommand sfdiskCommand(report, QStringLiteral("sfdisk"), { QStringLiteral("--part-type"), m_deviceNode, QString::number(partition.number()),
                QStringLiteral("C12A7328-F81F-11D2-BA4B-00A0C93EC93B") } );
        if (sfdiskCommand.run(-1) && sfdiskCommand.exitCode() == 0)
            return true;
        else
            return false;
    }
    if (flag == PartitionTable::FlagEsp && state == false)
        setPartitionSystemType(report, partition);

    if (flag == PartitionTable::FlagBiosGrub && state == true) {
        ExternalCommand sfdiskCommand(report, QStringLiteral("sfdisk"), { QStringLiteral("--part-type"), m_deviceNode, QString::number(partition.number()),
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
