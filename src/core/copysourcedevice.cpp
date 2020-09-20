/*
    SPDX-FileCopyrightText: 2008-2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2015 Teo Mrnjavac <teo@kde.org>
    SPDX-FileCopyrightText: 2016 Chantara Tith <tith.chantara@gmail.com>
    SPDX-FileCopyrightText: 2017-2018 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2018 Huzaifa Faruqui <huzaifafaruqui@gmail.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "core/copysourcedevice.h"

#include "backend/corebackend.h"
#include "backend/corebackendmanager.h"

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

QString CopySourceDevice::path() const
{
    return m_Device.deviceNode();
}
