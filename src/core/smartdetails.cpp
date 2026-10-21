/*
    SPDX-FileCopyrightText: 2026 Ramil Nurmanov <ramil2004nur@gmail.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "core/smartdetails.h"

#include "core/smartattributeparseddata.h"
#include "core/smartdiskinformation.h"
#include "core/smartparser.h"

#include <QDebug>

class SmartDetailsPrivate
{
public:
    QString m_DevicePath;
    bool m_Valid = false;
    bool m_Nvme = false;
    std::optional<quint64> m_Temp;
    std::optional<quint64> m_PoweredOn;
    std::optional<quint64> m_PowerCycles;
    std::optional<quint64> m_BadSectors;
    SmartNvmeHealth m_NvmeHealth;
};

SmartDetails::SmartDetails(const QString &devicePath) :
    d(std::make_unique<SmartDetailsPrivate>())
{
    d->m_DevicePath = devicePath;
    update();
}

SmartDetails::~SmartDetails() = default;

void SmartDetails::update()
{
    const QString devicePath = d->m_DevicePath;
    *d = SmartDetailsPrivate();
    d->m_DevicePath = devicePath;

    SmartParser parser(devicePath);

    if (!parser.init()) {
        qDebug() << "could not read SMART data of" << devicePath;
        return;
    }

    const SmartDiskInformation *disk = parser.diskInformation();

    if (!disk)
        return;

    d->m_Nvme = disk->isNvme();
    if (disk->temperatureValid())
        d->m_Temp = disk->temperature();
    if (disk->poweredOnValid())
        d->m_PoweredOn = disk->poweredOn();
    if (disk->powerCyclesValid())
        d->m_PowerCycles = disk->powerCycles();
    if (disk->badSectorsValid())
        d->m_BadSectors = disk->badSectors();
    d->m_NvmeHealth = disk->nvmeHealth();
    d->m_Valid = true;
}

const QString &SmartDetails::devicePath() const
{
    return d->m_DevicePath;
}

bool SmartDetails::isValid() const
{
    return d->m_Valid;
}

bool SmartDetails::isNvme() const
{
    return d->m_Nvme;
}

std::optional<quint64> SmartDetails::temp() const
{
    return d->m_Temp;
}

std::optional<quint64> SmartDetails::poweredOn() const
{
    return d->m_PoweredOn;
}

std::optional<quint64> SmartDetails::powerCycles() const
{
    return d->m_PowerCycles;
}

std::optional<quint64> SmartDetails::badSectors() const
{
    return d->m_BadSectors;
}

const SmartNvmeHealth &SmartDetails::nvmeHealth() const
{
    return d->m_NvmeHealth;
}
