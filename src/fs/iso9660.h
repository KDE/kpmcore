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

#if !defined(KPMCORE_ISO9660_H)

#define KPMCORE_ISO9660_H

#include "util/libpartitionmanagerexport.h"

#include "fs/filesystem.h"

class Report;

class QString;

namespace FS
{
/** A iso9660 file system.
    @author Andrius Štikonas <andrius@stikonas.eu>
 */
class LIBKPMCORE_EXPORT iso9660 : public FileSystem
{
public:
    iso9660(qint64 firstsector, qint64 lastsector, qint64 sectorsused, const QString& label);

};

}
#endif
