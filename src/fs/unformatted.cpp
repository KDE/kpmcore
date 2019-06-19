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

#include "fs/unformatted.h"

namespace FS
{
FileSystem::CommandSupportType unformatted::m_Create = FileSystem::cmdSupportFileSystem;

unformatted::unformatted(qint64 firstsector, qint64 lastsector, qint64 sectorsused, const QString& label, const QStringList& features) :
    FileSystem(firstsector, lastsector, sectorsused, label, features, FileSystem::Type::Unformatted)
{
}

bool unformatted::create(Report&, const QString&)
{
    return true;
}
}
