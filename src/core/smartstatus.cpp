/*************************************************************************
 *  Copyright (C) 2010 by Volker Lanz <vl@fidra.de>                      *
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

#include "core/smartstatus.h"

#include "core/smartparser.h"
#include "core/smartdiskinformation.h"
#include "core/smartattributeparseddata.h"

#include <KLocalizedString>

#include <QDebug>
#include <QString>
#include <QStringList>

#include <errno.h>

SmartStatus::SmartStatus(const QString &device_path) :
    m_DevicePath(device_path),
    m_InitSuccess(false),
    m_Status(false),
    m_ModelName(),
    m_Serial(),
    m_Firmware(),
    m_Overall(Bad),
    m_SelfTestStatus(Success),
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

    switch (disk->selfTestExecutionStatus()) {
    case SmartDiskInformation::SMART_SELF_TEST_EXECUTION_STATUS_ABORTED:
        setSelfTestStatus(Aborted);
        break;

    case SmartDiskInformation::SMART_SELF_TEST_EXECUTION_STATUS_INTERRUPTED:
        setSelfTestStatus(Interrupted);
        break;

    case SmartDiskInformation::SMART_SELF_TEST_EXECUTION_STATUS_FATAL:
        setSelfTestStatus(Fatal);
        break;

    case SmartDiskInformation::SMART_SELF_TEST_EXECUTION_STATUS_ERROR_UNKNOWN:
        setSelfTestStatus(ErrorUnknown);
        break;

    case SmartDiskInformation::SMART_SELF_TEST_EXECUTION_STATUS_ERROR_ELECTRICAL:
        setSelfTestStatus(ErrorEletrical);
        break;

    case SmartDiskInformation::SMART_SELF_TEST_EXECUTION_STATUS_ERROR_SERVO:
        setSelfTestStatus(ErrorServo);
        break;

    case SmartDiskInformation::SMART_SELF_TEST_EXECUTION_STATUS_ERROR_READ:
        setSelfTestStatus(ErrorRead);
        break;

    case SmartDiskInformation::SMART_SELF_TEST_EXECUTION_STATUS_ERROR_HANDLING:
        setSelfTestStatus(ErrorHandling);
        break;

    case SmartDiskInformation::SMART_SELF_TEST_EXECUTION_STATUS_INPROGRESS:
        setSelfTestStatus(InProgress);
        break;

    default:
    case SmartDiskInformation::SMART_SELF_TEST_EXECUTION_STATUS_SUCCESS_OR_NEVER:
        setSelfTestStatus(Success);
        break;

    }

    switch (disk->overall()) {
    case SmartDiskInformation::SMART_OVERALL_GOOD:
        setOverall(Good);
        break;

    case SmartDiskInformation::SMART_OVERALL_BAD_ATTRIBUTE_IN_THE_PAST:
        setOverall(BadPast);
        break;

    case SmartDiskInformation::SMART_OVERALL_BAD_SECTOR:
        setOverall(BadSectors);
        break;

    case SmartDiskInformation::SMART_OVERALL_BAD_ATTRIBUTE_NOW:
        setOverall(BadNow);
        break;

    case SmartDiskInformation::SMART_OVERALL_BAD_SECTOR_MANY:
        setOverall(BadSectorsMany);
        break;

    default:
    case SmartDiskInformation::SMART_OVERALL_BAD_STATUS:
        setOverall(Bad);
        break;

    }

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
                  QLocale().toString(celsius, 1), QLocale().toString(fahrenheit, 1));
}

QString SmartStatus::selfTestStatusToString(SmartStatus::SelfTestStatus s)
{
    switch (s) {
    case Aborted:
        return xi18nc("@item", "Aborted");

    case Interrupted:
        return xi18nc("@item", "Interrupted");

    case Fatal:
        return xi18nc("@item", "Fatal error");

    case ErrorUnknown:
        return xi18nc("@item", "Unknown error");

    case ErrorEletrical:
        return xi18nc("@item", "Electrical error");

    case ErrorServo:
        return xi18nc("@item", "Servo error");

    case ErrorRead:
        return xi18nc("@item", "Read error");

    case ErrorHandling:
        return xi18nc("@item", "Handling error");

    case InProgress:
        return xi18nc("@item", "Self test in progress");

    case Success:
    default:
        return xi18nc("@item", "Success");
    }

}

QString SmartStatus::overallAssessmentToString(Overall o)
{
    switch (o) {
    case Good:
        return xi18nc("@item", "Healthy");

    case BadPast:
        return xi18nc("@item", "Has been used outside of its design parameters in the past.");

    case BadSectors:
        return xi18nc("@item", "Has some bad sectors.");

    case BadNow:
        return xi18nc("@item", "Is being used outside of its design parameters right now.");

    case BadSectorsMany:
        return xi18nc("@item", "Has many bad sectors.");

    case Bad:
    default:
        return xi18nc("@item", "Disk failure is imminent. Backup all data!");
    }

}

void SmartStatus::addAttributes(QList<SmartAttributeParsedData> attr)
{
    m_Attributes.clear();

    for (const SmartAttributeParsedData &at : qAsConst(attr)) {
        SmartAttribute sm(at);
        m_Attributes.append(sm);
    }
}

