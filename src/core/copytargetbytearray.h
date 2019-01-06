/*************************************************************************
 *  Copyright (C) 2018 by Andrius Štikonas <andrius@stikonas.eu>         *
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

#ifndef KPMCORE_COPYTARGETBYTEARRAY_H
#define KPMCORE_COPYTARGETBYTEARRAY_H

#include "core/copytarget.h"

#include <QtGlobal>
#include <QByteArray>
#include <QString>

/** A file to copy to.

    Represents a target file to copy to. Used to back up a FileSystem to a file.

    @see CopySourceFile, CopyTargetDevice
    @author Volker Lanz <vl@fidra.de>
*/
class CopyTargetByteArray : public CopyTarget
{
public:
    CopyTargetByteArray(QByteArray& array);

public:
    bool open() override {
        return true;
    }

    QString path() const override {
        return QString();
    }

    qint64 firstByte() const override {
        return 0;    /**< @return always 0 for QByteArray */
    }
    qint64 lastByte() const override {
        return bytesWritten();    /**< @return the number of bytes written so far */
    }

    QByteArray& m_Array;
};

#endif
