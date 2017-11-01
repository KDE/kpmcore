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

#if !defined(KPMCORE_COPYSOURCEDEVICE_H)

#define KPMCORE_COPYSOURCEDEVICE_H

#include "core/copysource.h"
#include "util/libpartitionmanagerexport.h"

#include <QtGlobal>

class Device;
class CopyTarget;
class CoreBackendDevice;

/** A Device to copy from.

    Represents a Device to copy from. Used to copy a Partition to somewhere on the same or
    another Device or to backup its FileSystem to a file.
    @author Volker Lanz <vl@fidra.de>
 */
class CopySourceDevice : public CopySource
{
    Q_DISABLE_COPY(CopySourceDevice)

public:
    CopySourceDevice(Device& d, qint64 firstbyte, qint64 lastbyte);
    ~CopySourceDevice();

public:
    bool open() override;
    bool readData(QByteArray& buffer, qint64 readOffset, qint64 size) override;
    qint64 length() const override;
    bool overlaps(const CopyTarget& target) const override;

    qint64 firstByte() const override {
        return m_FirstByte;    /**< @return first byte to copy */
    }
    qint64 lastByte() const override {
        return m_LastByte;    /**< @return last byte to copy */
    }

    Device& device() {
        return m_Device;    /**< @return Device to copy from */
    }
    const Device& device() const {
        return m_Device;    /**< @return Device to copy from */
    }

protected:
    Device& m_Device;
    const qint64 m_FirstByte;
    const qint64 m_LastByte;
    CoreBackendDevice* m_BackendDevice
    ;
};

#endif
