/*
    SPDX-FileCopyrightText: 2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2014-2018 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2015-2016 Teo Mrnjavac <teo@kde.org>
    SPDX-FileCopyrightText: 2016 Chantara Tith <tith.chantara@gmail.com>
    SPDX-FileCopyrightText: 2019 Yuri Chornoivan <yurchor@ukr.net>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "core/partitionalignment.h"

#include "core/partition.h"
#include "core/partitiontable.h"
#include "core/device.h"
#include "core/diskdevice.h"

#include "fs/filesystem.h"

#include "util/globallog.h"

#include <KLocalizedString>

int PartitionAlignment::s_sectorAlignment = 2048;

qint64 PartitionAlignment::firstDelta(const Device& d, const Partition&, qint64 s)
{
    return s % sectorAlignment(d);
}

qint64 PartitionAlignment::lastDelta(const Device& d, const Partition&, qint64 s)
{
    return (s + 1) % sectorAlignment(d);
}

bool PartitionAlignment::isLengthAligned(const Device& d, const Partition& p)
{
    return p.length() % sectorAlignment(d) == 0;
}

/** Checks if the Partition is properly aligned to the PartitionTable's alignment requirements.

    Will print warning messages to GlobalLog if the Partition's first sector is not aligned and
    another one if the last sector is not aligned. This can be suppressed with setting the @p quiet to
    true.

    @see alignPartition(), canAlignToSector()

    @param d device the partition is on
    @param p the partition to check
    @param quiet if true, will not print warning
    @return true if properly aligned
*/
bool PartitionAlignment::isAligned(const Device& d, const Partition& p, bool quiet)
{
    return isAligned(d, p, p.firstSector(), p.lastSector(), quiet);
}

bool PartitionAlignment::isAligned(const Device& d, const Partition& p, qint64 newFirst, qint64 newLast, bool quiet)
{
    if (firstDelta(d, p, newFirst) && !quiet)
        Log(Log::Level::warning) << xi18nc("@info:status", "Partition <filename>%1</filename> is not properly aligned (first sector: %2, modulo: %3).", p.deviceNode(), newFirst, firstDelta(d, p, newFirst));

    if (lastDelta(d, p, newLast) && !quiet)
        Log(Log::Level::warning) << xi18nc("@info:status", "Partition <filename>%1</filename> is not properly aligned (last sector: %2, modulo: %3).", p.deviceNode(), newLast, lastDelta(d, p, newLast));

    return firstDelta(d, p, newFirst) == 0 && lastDelta(d, p, newLast) == 0;
}

/** @return the sector size to align the partition start and end to
*/
qint64 PartitionAlignment::sectorAlignment(const Device& d)
{
    Q_UNUSED(d)
    return s_sectorAlignment;
}

void PartitionAlignment::setSectorAlignment(int sectorAlignment)
{
    if (sectorAlignment > 0)
        s_sectorAlignment = sectorAlignment;
}

qint64 PartitionAlignment::alignedFirstSector(const Device& d, const Partition& p, qint64 s, qint64 min_first, qint64 max_first, qint64 min_length, qint64 max_length)
{
    if (firstDelta(d, p, s) == 0)
        return s;

    /** @todo Don't assume we always want to align to the front.
        Always trying to align to the front solves the problem that a partition does
        get too small to take another one that's copied to it, but it introduces
        a new bug: The user might create a partition aligned at the end of a device,
        extended partition or at the start of the next one, but we align to the back
        and leave some space in between.
    */
    // We always want to make the partition larger, not smaller. Making it smaller
    // might, in case it's a partition that another is being copied to, mean the partition
    // ends up too small. So try to move the start to the front first.
    s = s - firstDelta(d, p, s);

    while (s < d.partitionTable()->firstUsable() || s < min_first || (max_length > -1 && p.lastSector() - s + 1 > max_length))
        s += sectorAlignment(d);

    while (s > d.partitionTable()->lastUsable() || (max_first > -1 && s > max_first) || p.lastSector() - s + 1 < min_length)
        s -= sectorAlignment(d);

    return s;
}

qint64 PartitionAlignment::alignedLastSector(const Device& d, const Partition& p, qint64 s, qint64 min_last, qint64 max_last, qint64 min_length, qint64 max_length, qint64 original_length, bool original_aligned)
{
    if (lastDelta(d, p, s) == 0)
        return s;

    s = s + sectorAlignment(d) - lastDelta(d, p, s);

    // if we can retain the partition length exactly by aligning to the front, do that
    if (original_aligned && p.length() - original_length == lastDelta(d, p, s))
        s -= sectorAlignment(d);

    while (s < d.partitionTable()->firstUsable() || s < min_last || s - p.firstSector() + 1 < min_length)
        s += sectorAlignment(d);

    while (s > d.partitionTable()->lastUsable() || (max_last > -1 && s > max_last) || (max_length > -1 && s - p.firstSector() + 1 > max_length))
        s -= sectorAlignment(d);

    return s;
}
