/*
    SPDX-FileCopyrightText: 2008-2011 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2014-2017 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2015 Chris Campbell <c.j.campbell@ed.ac.uk>
    SPDX-FileCopyrightText: 2015-2016 Teo Mrnjavac <teo@kde.org>
    SPDX-FileCopyrightText: 2016 Chantara Tith <tith.chantara@gmail.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KPMCORE_HELPERS_H
#define KPMCORE_HELPERS_H

#include "core/partition.h"
#include "fs/filesystem.h"
#include "fs/luks.h"

#include "util/libpartitionmanagerexport.h"

class KAboutData;

class Partition;
class QString;

LIBKPMCORE_EXPORT void registerMetaTypes();
LIBKPMCORE_EXPORT bool caseInsensitiveLessThan(const QString& s1, const QString& s2);
LIBKPMCORE_EXPORT bool checkAccessibleDevices();
LIBKPMCORE_EXPORT bool isMounted(const QString& deviceNode);
LIBKPMCORE_EXPORT KAboutData aboutKPMcore();

/** Pointer to the file system (which might be inside LUKS container) contained in the partition
 * @param p Partition where we look for file system
 * @param fs inner FileSystem object
*/
template <typename T>
inline LIBKPMCORE_EXPORT void innerFS (const Partition* p, T& fs)
{
    Partition* partition = const_cast<Partition*>(p);
    if (p->roles().has(PartitionRole::Luks))
        fs = dynamic_cast<const T>(dynamic_cast<const FS::luks* const>(&p->fileSystem())->innerFS());
    else
        fs = dynamic_cast<const T>(&partition->fileSystem());
}

#endif
