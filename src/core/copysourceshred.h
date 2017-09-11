/*************************************************************************
 *  Copyright (C) 2010 by Volker Lanz <vl@fidra.de>                      *
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

#if !defined(KPMCORE_COPYSOURCESHRED_H)

#define KPMCORE_COPYSOURCESHRED_H

#include "core/copysource.h"

#include <QFile>

class CopyTarget;

/** A source for securely overwriting a partition (shredding).

    Represents a source of data (random or zeros) to copy from. Used to securely overwrite data on disk.

    @author Volker Lanz <vl@fidra.de>
*/
class CopySourceShred : public CopySource
{
public:
    CopySourceShred(qint64 size, qint32 sectorsize, bool randomShred);

public:
    bool open() override;
    bool readSectors(void* buffer, qint64 readOffset, qint64 numSectors) override;
    qint64 length() const override;

    qint32 sectorSize() const override {
        return m_SectorSize;    /**< @return the file's sector size */
    }
    bool overlaps(const CopyTarget&) const override {
        return false;    /**< @return false for shred source */
    }
    qint64 firstSector() const override {
        return 0;    /**< @return 0 for shred source */
    }
    qint64 lastSector() const override {
        return length();    /**< @return equal to length for shred source. @see length() */
    }

protected:
    QFile& sourceFile() {
        return m_SourceFile;
    }
    const QFile& sourceFile() const {
        return m_SourceFile;
    }
    qint64 size() const {
        return m_Size;
    }

private:
    qint64 m_Size;
    qint32 m_SectorSize;
    QFile m_SourceFile;
};

#endif
