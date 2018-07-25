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

#if !defined(KPMCORE_COPYSOURCE_H)

#define KPMCORE_COPYSOURCE_H

#include <QtGlobal>

class CopyTarget;
class QString;

/** Base class for something to copy from.

    Abstract base class for all copy sources. Used in combination with CopyTarget
    to implement moving, copying, backing up and restoring FileSystems.

    @see CopyTarget
    @author Volker Lanz <vl@fidra.de>
*/
class CopySource
{
    Q_DISABLE_COPY(CopySource)

protected:
    CopySource() {}
    virtual ~CopySource() {}

public:
    virtual bool open() = 0;
    virtual QString path() const = 0;
    virtual qint64 length() const = 0;
    virtual bool overlaps(const CopyTarget& target) const = 0;

    virtual qint64 firstByte() const = 0;
    virtual qint64 lastByte() const = 0;
};

#endif
