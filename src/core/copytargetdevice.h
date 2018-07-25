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

#if !defined(KPMCORE_COPYTARGETDEVICE_H)

#define KPMCORE_COPYTARGETDEVICE_H

#include "backend/corebackenddevice.h"
#include "core/copytarget.h"
#include "util/libpartitionmanagerexport.h"

#include <memory>

#include <QtGlobal>

class Device;
class CoreBackendDevice;

/** A Device to copy to.

    Represents a target Device to copy to. Used to copy a Partition to somewhere on the same
    or another Device or to restore a FileSystem from a file to a Partition.

    @see CopyTargetFile, CopySourceDevice

    @author Volker Lanz <vl@fidra.de>
*/
class CopyTargetDevice : public CopyTarget
{
    Q_DISABLE_COPY(CopyTargetDevice)

public:
    CopyTargetDevice(Device& d, qint64 firstbyte, qint64 lastbyte);

public:
    bool open() override;
    qint64 firstByte() const override {
        return m_FirstByte;    /**< @return the first byte to write to */
    }
    qint64 lastByte() const override {
        return m_LastByte;    /**< @return the last byte to write to */
    }

    Device& device() {
        return m_Device;    /**< @return the Device to write to */
    }
    const Device& device() const {
        return m_Device;    /**< @return the Device to write to */
    }
    QString path() const override;

protected:
    Device& m_Device;
    std::unique_ptr<CoreBackendDevice> m_BackendDevice;
    const qint64 m_FirstByte;
    const qint64 m_LastByte;
};

#endif
