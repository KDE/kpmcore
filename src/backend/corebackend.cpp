/*
    SPDX-FileCopyrightText: 2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2014-2020 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2015 Teo Mrnjavac <teo@kde.org>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "backend/corebackend.h"

#include "core/device.h"
#include "core/partitiontable.h"

#include "util/globallog.h"

#include <QDebug>

struct CoreBackendPrivate
{
    QString m_id, m_version;
};

CoreBackend::CoreBackend() :
    d(std::make_unique<CoreBackendPrivate>())
{
}

CoreBackend::~CoreBackend()
{
}

void CoreBackend::emitProgress(int i)
{
    Q_EMIT progress(i);
}

void CoreBackend::emitScanProgress(const QString& deviceNode, int i)
{
    Q_EMIT scanProgress(deviceNode, i);
}

void CoreBackend::setPartitionTableForDevice(Device& d, PartitionTable* p)
{
    d.setPartitionTable(p);
}

void CoreBackend::setPartitionTableMaxPrimaries(PartitionTable& p, qint32 max_primaries)
{
    p.setMaxPrimaries(max_primaries);
}

QString CoreBackend::id()
{
    return d->m_id;
}

QString CoreBackend::version()
{
    return d->m_version;
}

void CoreBackend::setId(const QString& id)
{
    d->m_id = id;
}

void CoreBackend::setVersion(const QString& version)
{
    d->m_version = version;
}
