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

#include "fs/luks2.h"

namespace FS
{

luks2::luks2(qint64 firstsector, qint64 lastsector, qint64 sectorsused, const QString& label)
    : luks(firstsector, lastsector, sectorsused, label, FileSystem::Luks2)
{
}

luks2::~luks2()
{
}

FileSystem::Type luks2::type() const
{
    if (m_isCryptOpen && m_innerFs)
        return m_innerFs->type();
    return FileSystem::Luks2;
}

}
