/*************************************************************************
 *  Copyright (C) 2018 by Caio Carvalho <caiojcarvalho@gmail.com>         *
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

#include "core/smartdiskinformation.h"
#include "core/smartattributeparseddata.h"

static quint64 u64log2(quint64 n);

SmartDiskInformation::SmartDiskInformation() :
    m_ModelName(QString()),
    m_FirmwareVersion(QString()),
    m_SerialNumber(QString()),
    m_Size(0),
    m_Temperature(0),
    m_BadSectors(0),
    m_PoweredOn(0),
    m_PowerCycles(0),
    m_SmartStatus(false),
    m_BadAttributeNow(false),
    m_BadAttributeInThePast(false),
    m_SelfTestExecutionStatus(SmartDiskInformation::SMART_SELF_TEST_EXECUTION_STATUS_SUCCESS_OR_NEVER),
    m_Overall(SmartDiskInformation::SMART_OVERALL_BAD_STATUS)
{

}

void SmartDiskInformation::updateBadSectors()
{
    SmartAttributeParsedData *reallocatedSectorCt;
    reallocatedSectorCt = findAttribute(5);

    SmartAttributeParsedData *currentPendingSector;
    currentPendingSector = findAttribute(197);

    if (!reallocatedSectorCt && !currentPendingSector)
        m_BadSectors = 0;
    else if (reallocatedSectorCt && currentPendingSector)
        m_BadSectors = reallocatedSectorCt->prettyValue() + currentPendingSector->prettyValue();
    else if (reallocatedSectorCt)
        m_BadSectors = reallocatedSectorCt->prettyValue();
    else
        m_BadSectors = currentPendingSector->prettyValue();
}

void SmartDiskInformation::updateOverall()
{
    if (!smartStatus()) {
        m_Overall = SMART_OVERALL_BAD_STATUS;
        return;
    }

    quint64 sector_threshold = u64log2(size() / 512) * 1024;

    if (badSectors() >= sector_threshold) {
        m_Overall = SMART_OVERALL_BAD_SECTOR_MANY;
        return;
    }

    validateBadAttributes();

    if (m_BadAttributeNow) {
        m_Overall = SMART_OVERALL_BAD_ATTRIBUTE_NOW;
        return;
    }

    if (badSectors() > 0) {
        m_Overall = SMART_OVERALL_BAD_SECTOR;
        return;
    }

    if (m_BadAttributeInThePast) {
        m_Overall = SMART_OVERALL_BAD_ATTRIBUTE_IN_THE_PAST;
        return;
    }

    m_Overall = SMART_OVERALL_GOOD;

}

bool SmartDiskInformation::updateTemperature()
{
    SmartAttributeParsedData *temperatureCelsius;
    SmartAttributeParsedData *temperatureCelsius2;
    SmartAttributeParsedData *airflowTemperatureCelsius;

    temperatureCelsius = findAttribute(231);
    temperatureCelsius2 = findAttribute(194);
    airflowTemperatureCelsius = findAttribute(190);

    if (temperatureCelsius != nullptr
            && temperatureCelsius->prettyUnit() == SmartAttributeParsedData::SMART_ATTRIBUTE_UNIT_MKELVIN) {
        m_Temperature = temperatureCelsius->prettyValue();
        return true;
    } else if (temperatureCelsius2 != nullptr
               && temperatureCelsius2->prettyUnit() == SmartAttributeParsedData::SMART_ATTRIBUTE_UNIT_MKELVIN) {
        m_Temperature = temperatureCelsius2->prettyValue();
        return true;
    } else if (airflowTemperatureCelsius != nullptr
               && airflowTemperatureCelsius->prettyUnit() ==
               SmartAttributeParsedData::SMART_ATTRIBUTE_UNIT_MKELVIN) {
        m_Temperature = airflowTemperatureCelsius->prettyValue();
        return true;
    }
    return false;
}

bool SmartDiskInformation::updatePowerOn()
{
    SmartAttributeParsedData *powerOnHours;
    SmartAttributeParsedData *powerOnSeconds;

    powerOnHours = findAttribute(9);
    powerOnSeconds = findAttribute(233);

    if (powerOnHours != nullptr
            && powerOnHours->prettyUnit() == SmartAttributeParsedData::SMART_ATTRIBUTE_UNIT_MSECONDS) {
        m_PoweredOn = powerOnHours->prettyValue();
        return true;
    } else if (powerOnSeconds != nullptr
               && powerOnSeconds->prettyUnit() == SmartAttributeParsedData::SMART_ATTRIBUTE_UNIT_MSECONDS) {
        m_PoweredOn = powerOnSeconds->prettyValue();
        return true;
    }
    return false;
}

bool SmartDiskInformation::updatePowerCycle()
{
    SmartAttributeParsedData *powerCycleCount;

    powerCycleCount = findAttribute(12);

    if (powerCycleCount != nullptr
            && powerCycleCount->prettyUnit() == SmartAttributeParsedData::SMART_ATTRIBUTE_UNIT_NONE) {
        m_PowerCycles = powerCycleCount->prettyValue();
        return true;
    }
    return false;
}

void SmartDiskInformation::validateBadAttributes()
{
    foreach (SmartAttributeParsedData attribute, m_Attributes) {
        if (attribute.prefailure()) {
            if (attribute.goodNowValid() && !attribute.goodNow())
                m_BadAttributeNow = true;
            if (attribute.goodInThePastValid() && !attribute.goodInThePast())
                m_BadAttributeInThePast = true;
        }
    }
}

SmartAttributeParsedData *SmartDiskInformation::findAttribute(quint32 id)
{
    SmartAttributeParsedData *attr = nullptr;
    foreach (SmartAttributeParsedData attribute, m_Attributes) {
        if (id == attribute.id()) {
            attr = new SmartAttributeParsedData(attribute);
            break;
        }
    }
    return attr;
}

static quint64 u64log2(quint64 n)
{
    quint64 r;

    if (n <= 1)
        return 0;

    r = 0;
    for (;;) {
        n = n >> 1;
        if (!n)
            return r;
        r++;
    }
    return 0;
}
