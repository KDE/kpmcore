/*
    SPDX-FileCopyrightText: 2017 Adriaan de Groot <groot@kde.org>
    SPDX-FileCopyrightText: 2017-2019 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2019 Shubham Jangra <aryan100jangid@gmail.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

// Lists devices

#include "helpers.h"

#include "backend/corebackend.h"
#include "backend/corebackendmanager.h"
#include "core/device.h"
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

    if (argc != 2) {
        i = std::make_unique<KPMCoreInitializer>();
        if (!i->isValid())
            return 1;
    } else {
        i = std::make_unique<KPMCoreInitializer>( argv[1] );
        if (!i->isValid())
            return 1;
    }

    auto backend = CoreBackendManager::self()->backend();

    if (!backend) {
        qWarning() << "Could not get backend.";
        return 1;
    }

    const auto devices = backend->scanDevices(ScanFlag::includeLoopback);
    qDebug() << "Found" << devices.length() << "devices.";

    for (const auto &pdev : devices) {
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

