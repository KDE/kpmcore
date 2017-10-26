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

#if !defined(KPMCORE_COPYTARGET_H)

#define KPMCORE_COPYTARGET_H

#include <QtGlobal>


/** Base class for something to copy to.

    Abstract base class for all copy targets. Used together with CopySource to
    implement moving, copying, restoring and backing up FileSystems.

    @see CopySource
    @author Volker Lanz <vl@fidra.de>
*/
class CopyTarget
{
    Q_DISABLE_COPY(CopyTarget)

protected:
    CopyTarget() : m_BytesWritten(0) {}
    virtual ~CopyTarget() {}

public:
    virtual bool open() = 0;
    virtual bool writeData(QByteArray& buffer, qint64 writeOffset) = 0;
    virtual qint64 firstByte() const = 0;
    virtual qint64 lastByte() const = 0;

    qint64 bytesWritten() const {
        return m_BytesWritten;
    }

protected:
    void setBytesWritten(qint64 s) {
        m_BytesWritten = s;
    }

private:
    qint64 m_BytesWritten;
};

#endif
