/*************************************************************************
 *  Copyright (C) 2018 by Caio Carvalho <caiojcarvalho@gmail.com>        *
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

#ifndef LINUXRAIDMEMBER_H
#define LINUXRAIDMEMBER_H

#include "util/libpartitionmanagerexport.h"

#include "fs/filesystem.h"

class Report;

class QString;

namespace FS
{
/** A linux_raid_member file system.
    @author Caio Carvalho <caiojcarvalho@gmail.com>
 */
class LIBKPMCORE_EXPORT linuxraidmember : public FileSystem
{
public:
    linuxraidmember(qint64 firstsector, qint64 lastsector, qint64 sectorsused, const QString& label);
};
}

#endif // LINUXRAIDMEMBER_H
