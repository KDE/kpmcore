/*
    SPDX-FileCopyrightText: 2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2016 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2015-2015 Teo Mrnjavac <teo@kde.org>
    SPDX-FileCopyrightText: 2015 Chris Campbell <c.j.campbell@ed.ac.uk>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KPMCORE_PARTITIONALIGNMENT_H
#define KPMCORE_PARTITIONALIGNMENT_H

#include "QtGlobal"

#include "util/libpartitionmanagerexport.h"

class Device;
class Partition;

class LIBKPMCORE_EXPORT PartitionAlignment
{
private:
    PartitionAlignment();

public:

    static bool isAligned(const Device& d, const Partition& p, bool quiet = false);
    static bool isAligned(const Device& d, const Partition& p, qint64 newFirst, qint64 newLast, bool quiet);

    static qint64 alignedFirstSector(const Device& d, const Partition& p, qint64 s, qint64 min_first, qint64 max_first, qint64 min_length, qint64 max_length);

    static qint64 alignedLastSector(const Device& d, const Partition& p, qint64 s, qint64 min_last, qint64 max_last, qint64 min_length, qint64 max_length, qint64 original_length = -1, bool original_aligned = false);

    static qint64 sectorAlignment(const Device& d);

    /** Sets the sector alignment multiplier for ALL devices henceforth except
     *  for devices that have a disklabel which aligns to cylinder boundaries.
     *  The default is 2048.
     *  This should probably be only set once on startup if necessary and not
     *  changed afterwards.
     */
    static void setSectorAlignment( int sectorAlignment );

    static qint64 firstDelta(const Device& d, const Partition& p, qint64 s);

    static qint64 lastDelta(const Device& d, const Partition& p, qint64 s);

    static bool isLengthAligned(const Device& d, const Partition& p);

private:
    static int s_sectorAlignment;
};

#endif
