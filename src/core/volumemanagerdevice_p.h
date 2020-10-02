/*
    SPDX-FileCopyrightText: 2018 Andrius Štikonas <andrius@stikonas.eu>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KPMCORE_VOLUMEMANAGERDEVICE_P_H
#define KPMCORE_VOLUMEMANAGERDEVICE_P_H

#include "core/device_p.h"

#include <QVector>

class Partition;

class VolumeManagerDevicePrivate : public DevicePrivate
{
public:
    QVector<const Partition*> m_PVs;
};

#endif
