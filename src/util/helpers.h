/*************************************************************************
 *  Copyright (C) 2008 by Volker Lanz <vl@fidra.de>                      *
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

#if !defined(KPMCORE_HELPERS_H)

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
