/*
    SPDX-FileCopyrightText: 2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2010 Yuri Chornoivan <yurchor@ukr.net>
    SPDX-FileCopyrightText: 2014-2020 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2015-2016 Teo Mrnjavac <teo@kde.org>
    SPDX-FileCopyrightText: 2018 Caio Jordão Carvalho <caiojcarvalho@gmail.com>
    SPDX-FileCopyrightText: 2019 Shubham Jangra <aryan100jangid@gmail.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "core/smartstatus.h"

#include "core/smartparser.h"
#include "core/smartdiskinformation.h"
#include "core/smartattributeparseddata.h"

#include <KLocalizedString>

#include <QDebug>
#include <QString>
#include <QStringList>

#include <errno.h>
#include <utility>

SmartStatus::SmartStatus(const QString &device_path) :
    m_DevicePath(device_path),
    m_InitSuccess(false),
    m_Status(false),
    m_ModelName(),
    m_Serial(),
    m_Firmware(),
    m_Overall(Overall::Bad),
    m_SelfTestStatus(SelfTestStatus::Success),
    m_Temp(0),
    m_BadSectors(0),
    m_PowerCycles(0),
    m_PoweredOn(0)
{
    update();
}

void SmartStatus::update()
{
    SmartParser parser(devicePath());

    if (!parser.init()) {
        qDebug() << "error during smart output parsing for " << devicePath() << ": " << strerror(errno);
        return;
    }

    SmartDiskInformation *disk;

    disk = parser.diskInformation();

    if (!disk)
        return;

    setStatus(disk->smartStatus());
    setModelName(disk->model());
    setFirmware(disk->firmware());
    setSerial(disk->serial());
    setSelfTestStatus(disk->selfTestExecutionStatus());
    setOverall(disk->overall());
    setTemp(disk->temperature());
    setBadSectors(disk->badSectors());
    setPoweredOn(disk->poweredOn());
    setPowerCycles(disk->powerCycles());
    addAttributes(disk->attributes());
    setInitSuccess(true);
}

QString SmartStatus::tempToString(quint64 mkelvin)
{
    const double celsius = (mkelvin - 273150.0) / 1000.0;
    const double fahrenheit = 9.0 * celsius / 5.0 + 32;
    return xi18nc("@item:intable degrees in Celsius and Fahrenheit", "%1° C / %2° F",
                  QLocale().toString(celsius, 'f', 0), QLocale().toString(fahrenheit, 'f', 0));
}

QString SmartStatus::selfTestStatusToString(SmartStatus::SelfTestStatus s)
{
    switch (s) {
    case SelfTestStatus::Aborted:
        return xi18nc("@item", "Aborted");

    case SelfTestStatus::Interrupted:
        return xi18nc("@item", "Interrupted");

    case SelfTestStatus::Fatal:
        return xi18nc("@item", "Fatal error");

    case SelfTestStatus::ErrorUnknown:
        return xi18nc("@item", "Unknown error");

    case SelfTestStatus::ErrorEletrical:
        return xi18nc("@item", "Electrical error");

    case SelfTestStatus::ErrorServo:
        return xi18nc("@item", "Servo error");

    case SelfTestStatus::ErrorRead:
        return xi18nc("@item", "Read error");

    case SelfTestStatus::ErrorHandling:
        return xi18nc("@item", "Handling error");

    case SelfTestStatus::InProgress:
        return xi18nc("@item", "Self test in progress");

    case SelfTestStatus::Success:
    default:
        return xi18nc("@item", "Success");
    }

}

QString SmartStatus::overallAssessmentToString(Overall o)
{
    switch (o) {
    case Overall::Good:
        return xi18nc("@item", "Healthy");

    case Overall::BadPast:
        return xi18nc("@item", "Has been used outside of its design parameters in the past.");

    case Overall::BadSectors:
        return xi18nc("@item", "Has some bad sectors.");

    case Overall::BadNow:
        return xi18nc("@item", "Is being used outside of its design parameters right now.");

    case Overall::BadSectorsMany:
        return xi18nc("@item", "Has many bad sectors.");

    case Overall::Bad:
    default:
        return xi18nc("@item", "Disk failure is imminent. Backup all data!");
    }

}

void SmartStatus::addAttributes(QList<SmartAttributeParsedData> attr)
{
    m_Attributes.clear();

    for (const SmartAttributeParsedData &at : std::as_const(attr)) {
        SmartAttribute sm(at);
        m_Attributes.append(sm);
    }
}

