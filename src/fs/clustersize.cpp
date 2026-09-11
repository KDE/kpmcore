/*
    SPDX-FileCopyrightText: 2026 Ramil Nurmanov <ramil2004nur@gmail.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "fs/clustersize.h"

#include "util/externalcommand.h"

#include <KLocalizedString>

#include <QMetaType>

namespace FS
{
namespace ClusterSize
{

namespace
{

bool isPowerOfTwo(qint64 v)
{
    return v > 0 && (v & (v - 1)) == 0;
}

bool typeSupportsSelection(FileSystem::Type t)
{
    switch (t) {
    case FileSystem::Type::Exfat:
    case FileSystem::Type::Fat16:
    case FileSystem::Type::Fat32:
    case FileSystem::Type::Ntfs:
        return true;
    default:
        return false;
    }
}

bool isIntegerVariant(const QVariant& v)
{
    switch (v.typeId()) {
    case QMetaType::Int:
    case QMetaType::UInt:
    case QMetaType::LongLong:
    case QMetaType::ULongLong:
        return true;
    default:
        return false;
    }
}

void byteRange(FileSystem::Type t, qint64 sectorSize, qint64& minBytes, qint64& maxBytes)
{
    const qint64 sector = sectorSize > 0 ? sectorSize : 512;

    switch (t) {
    case FileSystem::Type::Exfat:
        minBytes = qMax<qint64>(sector, 512);
        maxBytes = qint64(32) * 1024 * 1024;
        break;
    case FileSystem::Type::Ntfs:
        minBytes = qMax<qint64>(sector, 256);
        maxBytes = qint64(2) * 1024 * 1024;
        break;
    case FileSystem::Type::Fat16:
    case FileSystem::Type::Fat32:
        minBytes = sector;
        maxBytes = sector * 128;
        break;
    default:
        minBytes = 0;
        maxBytes = 0;
        break;
    }
}

qint64 powerOfTwoFeature(const QVariantMap& features, const QString& key, bool& ok)
{
    ok = true;
    if (!features.contains(key))
        return 0;

    const QVariant v = features.value(key);
    if (!isIntegerVariant(v)) {
        ok = false;
        return 0;
    }

    const qint64 n = v.toLongLong();
    if (n <= 0 || !isPowerOfTwo(n)) {
        ok = false;
        return 0;
    }
    return n;
}

}

QString errorFor(FileSystem::Type t, const QVariantMap& features, qint64 deviceSectorSize, qint64 fileSystemSizeInBytes)
{
    if (!features.contains(QStringLiteral("cluster-size")))
        return {};

    if (!typeSupportsSelection(t))
        return xi18nc("@info", "Choosing the cluster size is not supported for this file system.");

    const QVariant clusterValue = features.value(QStringLiteral("cluster-size"));
    if (!isIntegerVariant(clusterValue))
        return xi18nc("@info", "The cluster size must be given as an integer number of bytes.");

    const qint64 clusterBytes = clusterValue.toLongLong();
    if (clusterBytes <= 0)
        return xi18nc("@info", "The cluster size (%1 bytes) must be a positive number.", clusterBytes);
    if (!isPowerOfTwo(clusterBytes))
        return xi18nc("@info", "The cluster size (%1 bytes) must be a power of two.", clusterBytes);

    qint64 effectiveSector = deviceSectorSize > 0 ? deviceSectorSize : 512;

    if (t == FileSystem::Type::Fat16 || t == FileSystem::Type::Fat32) {
        bool ok = false;
        const qint64 featureSector = powerOfTwoFeature(features, QStringLiteral("sector-size"), ok);
        if (!ok)
            return xi18nc("@info", "The requested FAT sector size is not an integer power of two.");
        if (featureSector != 0) {
            if (featureSector < 512 || featureSector > 32768)
                return xi18nc("@info", "The requested FAT sector size (%1 bytes) must be between 512 and 32768.", featureSector);
            effectiveSector = featureSector;
        }

        const qint64 featureSpc = powerOfTwoFeature(features, QStringLiteral("sectors-per-cluster"), ok);
        if (!ok || featureSpc > 128)
            return xi18nc("@info", "The requested FAT sectors-per-cluster value is not an integer power of two from 1 to 128.");

        if (clusterBytes % effectiveSector != 0)
            return xi18nc("@info", "The cluster size (%1 bytes) must be a whole multiple of the sector size (%2 bytes).", clusterBytes, effectiveSector);

        const qint64 sectorsPerCluster = clusterBytes / effectiveSector;
        if (sectorsPerCluster < 1 || sectorsPerCluster > 128 || !isPowerOfTwo(sectorsPerCluster))
            return xi18nc("@info", "A cluster size of %1 bytes maps to %2 sectors of %3 bytes per cluster, which mkfs.fat cannot use (it must be a power of two from 1 to 128).", clusterBytes, sectorsPerCluster, effectiveSector);
        if (featureSpc != 0 && featureSpc != sectorsPerCluster)
            return xi18nc("@info", "The requested cluster size (%1 bytes) and sectors-per-cluster value (%2) do not agree.", clusterBytes, featureSpc);
    } else {
        if (clusterBytes % effectiveSector != 0)
            return xi18nc("@info", "The cluster size (%1 bytes) must be a whole multiple of the sector size (%2 bytes).", clusterBytes, effectiveSector);
    }

    qint64 lo = 0, hi = 0;
    byteRange(t, effectiveSector, lo, hi);
    if (clusterBytes < lo || clusterBytes > hi)
        return xi18nc("@info", "The cluster size (%1 bytes) is outside the range this file system supports (%2 to %3 bytes).", clusterBytes, lo, hi);

    if (fileSystemSizeInBytes > 0 && clusterBytes > fileSystemSizeInBytes)
        return xi18nc("@info", "The cluster size (%1 bytes) is larger than the whole file system (%2 bytes).", clusterBytes, fileSystemSizeInBytes);

    if ((t == FileSystem::Type::Fat16 || t == FileSystem::Type::Fat32) && fileSystemSizeInBytes > 0) {
        const qint64 clusterCountUpperBound = fileSystemSizeInBytes / clusterBytes;
        if (t == FileSystem::Type::Fat16 && clusterCountUpperBound < 4085)
            return xi18nc("@info", "A cluster size of %1 bytes leaves fewer than the 4085 data clusters a FAT16 file system needs. Choose a smaller cluster size.", clusterBytes);
        if (t == FileSystem::Type::Fat32 && clusterCountUpperBound < 65525)
            return xi18nc("@info", "A cluster size of %1 bytes leaves fewer than the 65525 data clusters a FAT32 file system needs. Choose a smaller cluster size.", clusterBytes);
    }

    return {};
}

QList<qint64> candidates(FileSystem::Type t, qint64 deviceSectorSize, qint64 fileSystemSizeInBytes)
{
    QList<qint64> result;

    if (!typeSupportsSelection(t))
        return result;

    qint64 lo = 0, hi = 0;
    byteRange(t, deviceSectorSize, lo, hi);
    if (lo <= 0 || hi <= 0)
        return result;

    QVariantMap probe;
    for (qint64 c = lo; c <= hi; c <<= 1) {
        probe.insert(QStringLiteral("cluster-size"), QVariant(c));
        if (errorFor(t, probe, deviceSectorSize, fileSystemSizeInBytes).isEmpty())
            result.append(c);
    }

    return result;
}

qint64 fromBootSector(const QByteArray& bootSector, FileSystem::Type t)
{
    if (bootSector.size() < 512)
        return -1;

    const auto u16le = [&bootSector](int o) { return qint64(quint8(bootSector.at(o))) | (qint64(quint8(bootSector.at(o + 1))) << 8); };

    switch (t) {
    case FileSystem::Type::Fat12:
    case FileSystem::Type::Fat16:
    case FileSystem::Type::Fat32: {
        const qint64 bytesPerSector = u16le(11);
        const qint64 sectorsPerCluster = quint8(bootSector.at(13));
        if (!isPowerOfTwo(bytesPerSector) || bytesPerSector < 512 || bytesPerSector > 32768)
            return -1;
        if (!isPowerOfTwo(sectorsPerCluster) || sectorsPerCluster > 128)
            return -1;
        return bytesPerSector * sectorsPerCluster;
    }
    case FileSystem::Type::Ntfs: {
        if (bootSector.mid(3, 4) != QByteArrayLiteral("NTFS"))
            return -1;
        const qint64 bytesPerSector = u16le(11);
        const quint32 spc = quint8(bootSector.at(13));
        if (!isPowerOfTwo(bytesPerSector) || bytesPerSector < 256)
            return -1;
        qint64 cluster = -1;
        if (spc >= 1 && spc <= 0x80)
            cluster = bytesPerSector * qint64(spc);
        else if (spc > 0x80) {
            const int shift = 0x100 - int(spc);
            if (shift < 9 || shift > 30)
                return -1;
            cluster = qint64(1) << shift;
        }
        return isPowerOfTwo(cluster) ? cluster : -1;
    }
    case FileSystem::Type::Exfat: {
        if (bootSector.mid(3, 8) != QByteArrayLiteral("EXFAT   "))
            return -1;
        const quint32 bytesPerSectorShift = quint8(bootSector.at(108));
        const quint32 sectorsPerClusterShift = quint8(bootSector.at(109));
        if (bytesPerSectorShift < 9 || bytesPerSectorShift > 12)
            return -1;
        if (sectorsPerClusterShift > 25 - bytesPerSectorShift)
            return -1;
        return qint64(1) << (bytesPerSectorShift + sectorsPerClusterShift);
    }
    default:
        return -1;
    }
}

qint64 fromBootSector(const QString& deviceNode, FileSystem::Type t)
{
    ExternalCommand cmd;
    return fromBootSector(cmd.readData(deviceNode, 0, 512), t);
}

}
}
