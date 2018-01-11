/*************************************************************************
 *  Copyright (C) 2018 by Caio Carvalho <caiojcarvalho@gmail.com>        *
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

#if !defined(KPMCORE_SMARTDISKINFORMATION_H)
#define SMARTDISKINFORMATION_H

#include <QList>
#include <QString>

class SmartAttributeParsedData;

class SmartDiskInformation
{
public:
    enum SmartSelfTestExecutionStatus {
        SMART_SELF_TEST_EXECUTION_STATUS_SUCCESS_OR_NEVER = 0,
        SMART_SELF_TEST_EXECUTION_STATUS_ABORTED = 1,
        SMART_SELF_TEST_EXECUTION_STATUS_INTERRUPTED = 2,
        SMART_SELF_TEST_EXECUTION_STATUS_FATAL = 3,
        SMART_SELF_TEST_EXECUTION_STATUS_ERROR_UNKNOWN = 4,
        SMART_SELF_TEST_EXECUTION_STATUS_ERROR_ELECTRICAL = 5,
        SMART_SELF_TEST_EXECUTION_STATUS_ERROR_SERVO = 6,
        SMART_SELF_TEST_EXECUTION_STATUS_ERROR_READ = 7,
        SMART_SELF_TEST_EXECUTION_STATUS_ERROR_HANDLING = 8,
        SMART_SELF_TEST_EXECUTION_STATUS_INPROGRESS = 15,
        _SMART_SELF_TEST_EXECUTION_STATUS_MAX
    };

    enum SmartOverall {
        SMART_OVERALL_GOOD,
        SMART_OVERALL_BAD_ATTRIBUTE_IN_THE_PAST,
        SMART_OVERALL_BAD_SECTOR,
        SMART_OVERALL_BAD_ATTRIBUTE_NOW,
        SMART_OVERALL_BAD_SECTOR_MANY,
        SMART_OVERALL_BAD_STATUS,
        _SMART_OVERALL_MAX
    };

public:
    SmartDiskInformation();

public:
    void updateBadSectors();

    void updateOverall();

    bool updateTemperature();

    bool updatePowerOn();

    bool updatePowerCycle();

    SmartAttributeParsedData *findAttribute(quint32 id);

public:
    const QString model() const
    {
        return m_ModelName;
    }

    const QString firmware() const
    {
        return m_FirmwareVersion;
    }

    const QString serial() const
    {
        return m_SerialNumber;
    }

    quint64 size() const
    {
        return m_Size;
    }

    bool smartStatus() const
    {
        return m_SmartStatus;
    }

    SmartSelfTestExecutionStatus selfTestExecutionStatus() const
    {
        return m_SelfTestExecutionStatus;
    }

    SmartOverall overall() const
    {
        return m_Overall;
    }

    quint64 temperature() const
    {
        return m_Temperature;
    }

    quint64 badSectors() const
    {
        return m_BadSectors;
    }

    quint64 poweredOn() const
    {
        return m_PoweredOn;
    }

    quint64 powerCycles() const
    {
        return m_PowerCycles;
    }

    QList<SmartAttributeParsedData> attributes() const
    {
        return m_Attributes;
    }

public:
    void setModel(QString modelName)
    {
        m_ModelName = modelName;
    }

    void setFirmware(QString firmware)
    {
        m_FirmwareVersion = firmware;
    }

    void setSerial(QString serial)
    {
        m_SerialNumber = serial;
    }

    void setSize(quint64 size)
    {
        m_Size = size;
    }

    void setPowerCycles(quint64 powerCycleCt)
    {
        m_PowerCycles = powerCycleCt;
    }

    void setSmartStatus(bool smartStatus)
    {
        m_SmartStatus = smartStatus;
    }

    void setSelfTestExecutionStatus(SmartSelfTestExecutionStatus status)
    {
        m_SelfTestExecutionStatus = status;
    }

    void addAttribute(SmartAttributeParsedData &attribute)
    {
        m_Attributes << attribute;
    }

protected:
    void validateBadAttributes();

private:
    QString m_ModelName;
    QString m_FirmwareVersion;
    QString m_SerialNumber;
    quint64 m_Size;
    quint64 m_Temperature;
    quint64 m_BadSectors;
    quint64 m_PoweredOn;
    quint64 m_PowerCycles;
    bool m_SmartStatus;
    bool m_BadAttributeNow;
    bool m_BadAttributeInThePast;
    SmartSelfTestExecutionStatus m_SelfTestExecutionStatus;
    SmartOverall m_Overall;
    QList<SmartAttributeParsedData> m_Attributes;

};

#endif // SMARTDISKINFORMATION_H
