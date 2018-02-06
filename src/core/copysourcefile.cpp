/*************************************************************************
 *  Copyright (C) 2008 by Volker Lanz <vl@fidra.de>                      *
 *  Copyright (C) 2016 by Andrius Štikonas <andrius@stikonas.eu>         *
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

#include "core/copysourcefile.h"

#include <QFile>
#include <QFileInfo>

/** Constructs a CopySourceFile from the given @p filename.
    @param filename filename of the file to copy from
*/
CopySourceFile::CopySourceFile(const QString& filename) :
    CopySource(),
    m_File(filename)
{
}

/** Opens the file.
    @return true on success
*/
bool CopySourceFile::open()
{
    return file().open(QIODevice::ReadOnly);
}

/** Returns the length of the file in bytes.
    @return length of the file in bytes.
*/
qint64 CopySourceFile::length() const
{
    return QFileInfo(file()).size();
}
