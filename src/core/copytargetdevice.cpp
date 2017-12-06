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

#include "core/copytargetdevice.h"

#include "backend/corebackend.h"
#include "backend/corebackendmanager.h"
#include "backend/corebackenddevice.h"

#include "core/device.h"


/** Constructs a device to copy to.
    @param d the Device to copy to
    @param firstbyte the first byte on the Device to write to
    @param lastbyte the last byte on the Device to write to
*/
CopyTargetDevice::CopyTargetDevice(Device& d, qint64 firstbyte, qint64 lastbyte) :
    CopyTarget(),
    m_Device(d),
    m_BackendDevice(nullptr),
    m_FirstByte(firstbyte),
    m_LastByte(lastbyte)
{
}

/** Destructs a CopyTargetDevice */
CopyTargetDevice::~CopyTargetDevice()
{
    delete m_BackendDevice;
}

/** Opens a CopyTargetDevice for writing to.
    @return true on success
*/
bool CopyTargetDevice::open()
{
    m_BackendDevice = CoreBackendManager::self()->backend()->openDeviceExclusive(device());
    return m_BackendDevice != nullptr;
}

/** Writes the given number of bytes to the Device.

    Note that @p writeOffset must be greater or equal than zero.

    @param buffer the data to write
    @param writeOffset where to start writing on the Device
    @return true on success
*/
bool CopyTargetDevice::writeData(QByteArray& buffer, qint64 writeOffset)
{
    Q_ASSERT(writeOffset >= 0);
    bool rval = m_BackendDevice->writeData(buffer, writeOffset);

    if (rval)
        setBytesWritten(bytesWritten() + buffer.size());

    return rval;
}
