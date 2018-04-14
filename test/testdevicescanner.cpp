/*************************************************************************
 *  Copyright 2017 by Adriaan de Groot <groot@kde.org>                   *
 *  Copyright 2017 by Andrius Štikonas <andrius@stikonas.eu>             *
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

//  SPDX-License-Identifier: GPL-3.0+

// Lists devices

#include "helpers.h"

#include "backend/corebackend.h"
#include "backend/corebackendmanager.h"
#include "core/device.h"
#include "core/devicescanner.h"
#include "core/operationstack.h"
#include "core/partition.h"
#include "util/capacity.h"

#include <QCoreApplication>
#include <QDebug>
#include <QList>

#include <memory>

using PartitionList = QList<Partition *>;

// Recursive helper for flatten(), adds partitions that
// are children of @p n to the list @p l.
void _flatten(PartitionList& l, PartitionNode *n)
{
    for (const auto &p : n->children()) {
        l.append(p);

        if (p->roles().has(PartitionRole::Extended)) {
            _flatten(l, p);
        }
    }
}

/**
 * Recursively walk the partition table and collect all the partitions
 * in it (also in extended partitions). Produces a sorted list.
 */
PartitionList flatten(PartitionTable *table)
{
    PartitionList l;
    _flatten(l, table);
    std::sort(l.begin(), l.end(),
                [](const Partition* p1, const Partition* p2) { return p1->number() < p2->number(); });
    return l;
}

int main( int argc, char **argv )
{
    QCoreApplication app(argc, argv);
    std::unique_ptr<KPMCoreInitializer> i;

    if (argc != 2)
    {
        i = std::make_unique<KPMCoreInitializer>();
        if (!i->isValid())
            return 1;
    }
    else
    {
        i = std::make_unique<KPMCoreInitializer>( argv[1] );
        if (!i->isValid())
            return 1;
    }

    auto backend = CoreBackendManager::self()->backend();
    if (!backend)
    {
        qWarning() << "Could not get backend.";
        return 1;
    }

     // First create operationStack with another QObject as parent.
    OperationStack *operationStack = new OperationStack();
    DeviceScanner *deviceScanner = new DeviceScanner(nullptr, *operationStack);
    deviceScanner->scan();

    const auto devices = operationStack->previewDevices();
    qDebug() << "Found" << devices.length() << "devices.";
    for (const auto pdev : devices)
    {
        qDebug() << "Device @" << (void *)pdev;
        qDebug() << "  " << pdev->prettyName();

        const auto partitiontable = pdev->partitionTable();
        qDebug() << "    Partition Table @" << (void *)partitiontable << '('
            << (partitiontable ? partitiontable->typeName() : QLatin1String("null")) << ')';

        const auto partitionlist = flatten(partitiontable);
        for (const auto &p : partitionlist)
            qDebug() << "      "
            << p->number()
            << p->partitionPath()
            << p->label()
            << Capacity::formatByteSize(p->capacity())
            << p->fileSystem().name();
    }

    return 0;
}

