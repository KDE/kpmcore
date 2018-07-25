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

#if !defined(KPMCORE_COPYSOURCEFILE_H)

#define KPMCORE_COPYSOURCEFILE_H

#include "core/copysource.h"

#include <QtGlobal>
#include <QFile>

class QString;
class CopyTarget;

/** A file to copy from.

    Represents a file to copy from. Used to restore a FileSystem from a backup file.

    @author Volker Lanz <vl@fidra.de>
*/
class CopySourceFile : public CopySource
{
public:
    CopySourceFile(const QString& filename);

public:
    bool open() override;
    qint64 length() const override;

    bool overlaps(const CopyTarget&) const override {
        return false;    /**< @return false for file */
    }
    qint64 firstByte() const override {
        return 0;    /**< @return 0 for file */
    }
    qint64 lastByte() const override {
        return length();    /**< @return equal to length for file. @see length() */
    }
    QString path() const override {
        return m_File.fileName();
    }

protected:
    QFile& file() {
        return m_File;
    }
    const QFile& file() const {
        return m_File;
    }

protected:
    QFile m_File;
};

#endif
