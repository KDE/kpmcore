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

#include <KLocalizedString>

#include <QDebug>
#include <QString>
#include <QStringList>

#include <atasmart.h>
#include <errno.h>

SmartStatus::SmartStatus(const QString& device_path) :
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
    SkDisk* skDisk = nullptr;
    SkBool skSmartStatus = false;
    uint64_t mkelvin = 0;
    uint64_t skBadSectors = 0;
    uint64_t skPoweredOn = 0;
    uint64_t skPowerCycles = 0;

    if (sk_disk_open(devicePath().toLocal8Bit().constData(), &skDisk) < 0) {
        qDebug() << "smart disk open failed for " << devicePath() << ": " << strerror(errno);
        return;
    }

    if (sk_disk_smart_status(skDisk, &skSmartStatus) < 0) {
        qDebug() << "getting smart status failed for " << devicePath() << ": " << strerror(errno);
        sk_disk_free(skDisk);
        return;
    }

    setStatus(skSmartStatus);

    if (sk_disk_smart_read_data(skDisk) < 0) {
        qDebug() << "reading smart data failed for " << devicePath() << ": " << strerror(errno);
        sk_disk_free(skDisk);
        return;
    }

    const SkIdentifyParsedData* skIdentify;

    if (sk_disk_identify_parse(skDisk, &skIdentify) < 0)
        qDebug() << "getting identify data failed for " <<  devicePath() << ": " << strerror(errno);
    else {
        setModelName(QString::fromLocal8Bit(skIdentify->model));
        setFirmware(QString::fromLocal8Bit(skIdentify->firmware));
        setSerial(QString::fromLocal8Bit(skIdentify->serial));
    }

    const SkSmartParsedData* skParsed;
    if (sk_disk_smart_parse(skDisk, &skParsed) < 0)
        qDebug() << "parsing disk smart data failed for " <<  devicePath() << ": " << strerror(errno);
    else {
        switch (skParsed->self_test_execution_status) {
        case SK_SMART_SELF_TEST_EXECUTION_STATUS_ABORTED:
            setSelfTestStatus(Aborted);
            break;

        case SK_SMART_SELF_TEST_EXECUTION_STATUS_INTERRUPTED:
            setSelfTestStatus(Interrupted);
            break;

        case SK_SMART_SELF_TEST_EXECUTION_STATUS_FATAL:
            setSelfTestStatus(Fatal);
            break;

        case SK_SMART_SELF_TEST_EXECUTION_STATUS_ERROR_UNKNOWN:
            setSelfTestStatus(ErrorUnknown);
            break;

        case SK_SMART_SELF_TEST_EXECUTION_STATUS_ERROR_ELECTRICAL:
            setSelfTestStatus(ErrorEletrical);
            break;

        case SK_SMART_SELF_TEST_EXECUTION_STATUS_ERROR_SERVO:
            setSelfTestStatus(ErrorServo);
            break;

        case SK_SMART_SELF_TEST_EXECUTION_STATUS_ERROR_READ:
            setSelfTestStatus(ErrorRead);
            break;

        case SK_SMART_SELF_TEST_EXECUTION_STATUS_ERROR_HANDLING:
            setSelfTestStatus(ErrorHandling);
            break;

        case SK_SMART_SELF_TEST_EXECUTION_STATUS_INPROGRESS:
            setSelfTestStatus(InProgress);
            break;

        default:
        case SK_SMART_SELF_TEST_EXECUTION_STATUS_SUCCESS_OR_NEVER:
            setSelfTestStatus(Success);
            break;
        }
    }

    SkSmartOverall overall;

    if (sk_disk_smart_get_overall(skDisk, &overall) < 0)
        qDebug() << "getting status failed for " <<  devicePath() << ": " << strerror(errno);
    else {
        switch (overall) {
        case SK_SMART_OVERALL_GOOD:
            setOverall(Good);
            break;

        case SK_SMART_OVERALL_BAD_ATTRIBUTE_IN_THE_PAST:
            setOverall(BadPast);
            break;

        case SK_SMART_OVERALL_BAD_SECTOR:
            setOverall(BadSectors);
            break;

        case SK_SMART_OVERALL_BAD_ATTRIBUTE_NOW:
            setOverall(BadNow);
            break;

        case SK_SMART_OVERALL_BAD_SECTOR_MANY:
            setOverall(BadSectorsMany);
            break;

        default:
        case SK_SMART_OVERALL_BAD_STATUS:
            setOverall(Bad);
            break;
        }
    }

    if (sk_disk_smart_get_temperature(skDisk, &mkelvin) < 0)
        qDebug() << "getting temp failed for " <<  devicePath() << ": " << strerror(errno);
    else
        setTemp(mkelvin);

    if (sk_disk_smart_get_bad(skDisk, &skBadSectors) < 0)
        qDebug() << "getting bad sectors failed for " <<  devicePath() << ": " << strerror(errno);
    else
        setBadSectors(skBadSectors);

    if (sk_disk_smart_get_power_on(skDisk, &skPoweredOn) < 0)
        qDebug() << "getting powered on time failed for " <<  devicePath() << ": " << strerror(errno);
    else
        setPoweredOn(skPoweredOn);

    if (sk_disk_smart_get_power_cycle(skDisk, &skPowerCycles) < 0)
        qDebug() << "getting power cycles failed for " <<  devicePath() << ": " << strerror(errno);
    else
        setPowerCycles(skPowerCycles);

    m_Attributes.clear();

    sk_disk_smart_parse_attributes(skDisk, callback, this);

    sk_disk_free(skDisk);
    setInitSuccess(true);
}

QString SmartStatus::tempToString(quint64 mkelvin)
{
    const double celsius = (mkelvin - 273150.0) / 1000.0;
    const double fahrenheit = 9.0 * celsius / 5.0 + 32;
    return xi18nc("@item:intable degrees in Celsius and Fahrenheit", "%1° C / %2° F", QLocale().toString(celsius, 1), QLocale().toString(fahrenheit, 1));
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

void SmartStatus::callback(SkDisk*, const SkSmartAttributeParsedData* a, void* user_data)
{
    SmartStatus* self = reinterpret_cast<SmartStatus*>(user_data);

    SmartAttribute sm(a);
    self->m_Attributes.append(sm);
}

