/*
    SPDX-FileCopyrightText: 2008-2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2015 Teo Mrnjavac <teo@kde.org>
    SPDX-FileCopyrightText: 2016 Chantara Tith <tith.chantara@gmail.com>
    SPDX-FileCopyrightText: 2017-2018 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2018 Huzaifa Faruqui <huzaifafaruqui@gmail.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "core/copytargetdevice.h"

#include "backend/corebackend.h"
#include "backend/corebackendmanager.h"

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

/** Opens a CopyTargetDevice for writing to.
    @return true on success
*/
bool CopyTargetDevice::open()
{
    m_BackendDevice = CoreBackendManager::self()->backend()->openDeviceExclusive(device());
    return m_BackendDevice != nullptr;
}

QString CopyTargetDevice::path() const
{
    return m_Device.deviceNode();
}
