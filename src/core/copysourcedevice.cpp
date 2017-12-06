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

#include "core/copysourcedevice.h"

#include "backend/corebackend.h"
#include "backend/corebackendmanager.h"
#include "backend/corebackenddevice.h"

#include "core/copytarget.h"
#include "core/copytargetdevice.h"
#include "core/device.h"

/** Constructs a CopySource on the given Device
    @param d Device from which to copy
    @param firstbyte the first byte that will be copied
    @param lastbyte the last byte that will be copied
*/
CopySourceDevice::CopySourceDevice(Device& d, qint64 firstbyte, qint64 lastbyte) :
    CopySource(),
    m_Device(d),
    m_FirstByte(firstbyte),
    m_LastByte(lastbyte),
    m_BackendDevice(nullptr)
{
}

/** Destructs a CopySourceDevice */
CopySourceDevice::~CopySourceDevice()
{
    delete m_BackendDevice;
}

/** Opens the Device
    @return true if the Device could be successfully opened
*/
bool CopySourceDevice::open()
{
    m_BackendDevice = CoreBackendManager::self()->backend()->openDeviceExclusive(device());
    return m_BackendDevice != nullptr;
}

/** Returns the length of this CopySource
    @return length of the copy source
*/
qint64 CopySourceDevice::length() const
{
    return lastByte() - firstByte() + 1;
}

/** Reads a given number of bytes from the Device into the given buffer.

    Note that @p readOffset must be greater or equal than zero.

    @param buffer the buffer to store the read bytes in
    @param readOffset the offset to begin reading
    @param size the number of bytes to read

    @return true if successful
*/
bool CopySourceDevice::readData(QByteArray& buffer, qint64 readOffset, qint64 size)
{
    Q_ASSERT(readOffset >= 0);
    return m_BackendDevice->readData(buffer, readOffset, size);
}

/** Checks if this CopySourceDevice overlaps with the given CopyTarget
    @param target the CopyTarget to check overlapping with
    @return true if overlaps
*/
bool CopySourceDevice::overlaps(const CopyTarget& target) const
{
    try {
        const CopyTargetDevice& t = dynamic_cast<const CopyTargetDevice&>(target);

        if (device().deviceNode() != t.device().deviceNode())
            return false;

        // overlapping at the front?
        if (firstByte() <= t.firstByte() && lastByte() >= t.firstByte())
            return true;

        // overlapping at the back?
        if (firstByte() <= t.lastByte() && lastByte() >= t.lastByte())
            return true;
    } catch (...) {
    }

    return false;
}
