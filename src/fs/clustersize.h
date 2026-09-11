/*
    SPDX-FileCopyrightText: 2026 Ramil Nurmanov <ramil2004nur@gmail.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KPMCORE_CLUSTERSIZE_H
#define KPMCORE_CLUSTERSIZE_H

#include "fs/filesystem.h"

#include <QByteArray>
#include <QList>
#include <QString>
#include <QVariant>
#include <QtGlobal>

namespace FS
{
namespace ClusterSize
{

QString errorFor(FileSystem::Type t, const QVariantMap& features, qint64 deviceSectorSize, qint64 fileSystemSizeInBytes);

QList<qint64> candidates(FileSystem::Type t, qint64 deviceSectorSize, qint64 fileSystemSizeInBytes);

qint64 fromBootSector(const QByteArray& bootSector, FileSystem::Type t);

qint64 fromBootSector(const QString& deviceNode, FileSystem::Type t);

}
}

#endif
