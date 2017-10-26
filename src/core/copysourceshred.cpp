/*************************************************************************
 *  Copyright (C) 2010 by Volker Lanz <vl@fidra.de>                      *
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

#include "core/copysourceshred.h"

/** Constructs a CopySourceShred with the given @p size
    @param s the size the copy source will (pretend to) have
*/
CopySourceShred::CopySourceShred(qint64 s, bool randomShred) :
    CopySource(),
    m_Size(s),
    m_SourceFile(randomShred ? QStringLiteral("/dev/urandom") : QStringLiteral("/dev/zero"))
{
}

/** Opens the shred source.
    @return true on success
*/
bool CopySourceShred::open()
{
    return sourceFile().open(QIODevice::ReadOnly);
}

/** Returns the length of the source in bytes.
    @return length of the source in bytes.
*/
qint64 CopySourceShred::length() const
{
    return size();
}

/** Reads the given number of bytes from the source into the given buffer.
    @param buffer buffer to store the data read in
    @param readOffset offset where to begin reading (unused)
    @param size the number of bytes to read
    @return true on success
*/
bool CopySourceShred::readData(QByteArray& buffer, qint64 readOffset, qint64 size)
{
    Q_UNUSED(readOffset);

    buffer = sourceFile().read(size);
    return !buffer.isEmpty();
}
