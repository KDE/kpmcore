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

#include "core/copytargetfile.h"

/** Constructs a file to write to.
    @param filename name of the file to write to
*/
CopyTargetFile::CopyTargetFile(const QString& filename) :
    CopyTarget(),
    m_File(filename)
{
}

/** Opens the file for writing.
    @return true on success
*/
bool CopyTargetFile::open()
{
    return file().open(QIODevice::WriteOnly | QIODevice::Truncate);
}

/** Writes the given number of bytes from the given buffer to the file.
    @param buffer the data to write
    @param writeOffset where in the file to start writing
    @return true on success
*/
bool CopyTargetFile::writeData(QByteArray& buffer, qint64 writeOffset)
{
    if (!file().seek(writeOffset))
        return false;

    bool rval = file().write(buffer) == buffer.size();

    if (rval)
        setBytesWritten(bytesWritten() + buffer.size());

    return rval;
}
