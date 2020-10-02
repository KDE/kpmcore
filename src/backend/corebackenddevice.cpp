/*
    SPDX-FileCopyrightText: 2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2017 Andrius Štikonas <andrius@stikonas.eu>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "backend/corebackenddevice.h"

CoreBackendDevice::CoreBackendDevice(const QString& deviceNode) :
    m_DeviceNode(deviceNode),
    m_Exclusive(false)
{
}
