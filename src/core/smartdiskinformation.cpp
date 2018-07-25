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

#include <memory>

static quint64 u64log2(quint64 n);

/** Creates a new SmartDiskInformationObject */
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
    m_SelfTestExecutionStatus(SmartStatus::SelfTestStatus::Success),
    m_Overall(SmartStatus::Overall::Bad)
{
}

/** Update the number of bad sectors based on reallocated sector count and current pending sector attributes data */
void SmartDiskInformation::updateBadSectors()
{
    std::unique_ptr<SmartAttributeParsedData> reallocatedSectorCt(findAttribute(5));
    std::unique_ptr<SmartAttributeParsedData> currentPendingSector(findAttribute(197));

    if (!reallocatedSectorCt && !currentPendingSector)
        m_BadSectors = 0;
    else if (reallocatedSectorCt && currentPendingSector)
        m_BadSectors = reallocatedSectorCt->prettyValue() + currentPendingSector->prettyValue();
    else if (reallocatedSectorCt)
        m_BadSectors = reallocatedSectorCt->prettyValue();
    else
        m_BadSectors = currentPendingSector->prettyValue();
}

/** Update SMART overall data based on the quantity of bad sectors and the status of SMART attributes */
void SmartDiskInformation::updateOverall()
{
    if (!smartStatus()) {
        m_Overall = SmartStatus::Overall::Bad;
        return;
    }

    quint64 sector_threshold = u64log2(size() / 512) * 1024;

    if (badSectors() >= sector_threshold) {
        m_Overall = SmartStatus::Overall::BadSectorsMany;
        return;
    }

    validateBadAttributes();

    if (m_BadAttributeNow) {
        m_Overall = SmartStatus::Overall::BadNow;
        return;
    }

    if (badSectors() > 0) {
        m_Overall = SmartStatus::Overall::BadSectors;
        return;
    }

    if (m_BadAttributeInThePast) {
        m_Overall = SmartStatus::Overall::BadPast;
        return;
    }

    m_Overall = SmartStatus::Overall::Good;
}

/** Update the temperature value based on SMART attributes
    @return a boolean representing the status of the operation
*/
bool SmartDiskInformation::updateTemperature()
{
    std::unique_ptr<SmartAttributeParsedData> temperatureCelsius(findAttribute(231));
    std::unique_ptr<SmartAttributeParsedData> temperatureCelsius2(findAttribute(194));
    std::unique_ptr<SmartAttributeParsedData> airflowTemperatureCelsius(findAttribute(190));

    if (temperatureCelsius != nullptr
            && temperatureCelsius->prettyUnit() == SmartAttributeUnit::Milikelvin) {
        m_Temperature = temperatureCelsius->prettyValue();
        return true;
    } else if (temperatureCelsius2 != nullptr
               && temperatureCelsius2->prettyUnit() == SmartAttributeUnit::Milikelvin) {
        m_Temperature = temperatureCelsius2->prettyValue();
        return true;
    } else if (airflowTemperatureCelsius != nullptr
               && airflowTemperatureCelsius->prettyUnit() ==
               SmartAttributeUnit::Milikelvin) {
        m_Temperature = airflowTemperatureCelsius->prettyValue();
        return true;
    }
    return false;
}

/** Update the powered on value based on SMART attributes
    @return a boolean representing the status of the operation
*/
bool SmartDiskInformation::updatePowerOn()
{
    std::unique_ptr<SmartAttributeParsedData> powerOnHours(findAttribute(9));
    std::unique_ptr<SmartAttributeParsedData> powerOnSeconds(findAttribute(233));

    if (powerOnHours != nullptr
            && powerOnHours->prettyUnit() == SmartAttributeUnit::Miliseconds) {
        m_PoweredOn = powerOnHours->prettyValue();
        return true;
    } else if (powerOnSeconds != nullptr
               && powerOnSeconds->prettyUnit() == SmartAttributeUnit::Miliseconds) {
        m_PoweredOn = powerOnSeconds->prettyValue();
        return true;
    }
    return false;
}

/** Update the power cycles value based on SMART attributes
    @return a boolean representing the status of the operation
*/
bool SmartDiskInformation::updatePowerCycle()
{
    std::unique_ptr<SmartAttributeParsedData> powerCycleCount(findAttribute(12));

    if (powerCycleCount != nullptr
            && powerCycleCount->prettyUnit() == SmartAttributeUnit::None) {
        m_PowerCycles = powerCycleCount->prettyValue();
        return true;
    }
    return false;
}

/** Validate disk attributes status */
void SmartDiskInformation::validateBadAttributes()
{
    for (const SmartAttributeParsedData &attribute : qAsConst(m_Attributes)) {
        if (attribute.prefailure()) {
            if (attribute.goodNowValid() && !attribute.goodNow())
                m_BadAttributeNow = true;
            if (attribute.goodInThePastValid() && !attribute.goodInThePast())
                m_BadAttributeInThePast = true;
        }
    }
}

/** Search for a attribute based on its id number
    @return a reference to the attribute
*/
SmartAttributeParsedData *SmartDiskInformation::findAttribute(quint32 id)
{
    SmartAttributeParsedData *attr = nullptr;
    for (const SmartAttributeParsedData &attribute : qAsConst(m_Attributes)) {
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
