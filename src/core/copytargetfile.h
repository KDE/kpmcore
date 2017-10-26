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

#if !defined(KPMCORE_COPYTARGETFILE_H)

#define KPMCORE_COPYTARGETFILE_H

#include "core/copytarget.h"

#include <QtGlobal>
#include <QFile>

class QString;

/** A file to copy to.

    Repesents a target file to copy to. Used to back up a FileSystem to a file.

    @see CopySourceFile, CopyTargetDevice
    @author Volker Lanz <vl@fidra.de>
*/
class CopyTargetFile : public CopyTarget
{
public:
    CopyTargetFile(const QString& filename);

public:
    bool open() override;
    bool writeData(QByteArray& buffer, qint64 writeOffset) override;

    qint64 firstByte() const override {
        return 0;    /**< @return always 0 for a file */
    }
    qint64 lastByte() const override {
        return bytesWritten();    /**< @return the number of bytes written so far */
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
