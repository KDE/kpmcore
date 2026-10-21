/*
    SPDX-FileCopyrightText: 2018 Caio Jordão Carvalho <caiojcarvalho@gmail.com>
    SPDX-FileCopyrightText: 2018-2019 Andrius Štikonas <andrius@stikonas.eu>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KPMCORE_SMARTDISKINFORMATION_H
#define KPMCORE_SMARTDISKINFORMATION_H

#include "core/smartnvmehealth.h"
#include "core/smartstatus.h"

#include <QList>
#include <QString>

class SmartAttributeParsedData;

/** Disk information retrieved by SMART.

    It includes a list with your SMART attributes.

    @author Caio Jordão Carvalho <caiojcarvalho@gmail.com>
*/
class SmartDiskInformation
{
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
        return m_ModelName;   /**< @return the disk model name */
    }

    const QString firmware() const
    {
        return m_FirmwareVersion;    /**< @return the disk firmware version */
    }

    const QString serial() const
    {
        return m_SerialNumber;    /**< @return the disk serial number */
    }

    quint64 sectors() const
    {
        return m_Sectors;    /**< @return disk size */
    }

    bool smartStatus() const
    {
        return m_SmartStatus;    /**< @return a boolean representing SMART status */
    }

    SmartStatus::SelfTestStatus selfTestExecutionStatus() const
    {
        return m_SelfTestExecutionStatus;    /**< @return SMART self execution status */
    }

    SmartStatus::Overall overall() const
    {
        return m_Overall;    /**< @return SMART overall status */
    }

    quint64 temperature() const
    {
        return m_Temperature;    /**< @return disk temperature in kelvin */
    }

    bool temperatureValid() const
    {
        return m_TemperatureValid;
    }

    quint64 badSectors() const
    {
        return m_BadSectors;    /**< @return the number of bad sectors */
    }

    bool badSectorsValid() const
    {
        return !m_Nvme;
    }

    quint64 poweredOn() const
    {
        return m_PoweredOn;    /**< @return quantity of time that device is powered on */
    }

    bool poweredOnValid() const
    {
        return m_PoweredOnValid;
    }

    quint64 powerCycles() const
    {
        return m_PowerCycles;    /**< @return quantity of power cycles */
    }

    bool powerCyclesValid() const
    {
        return m_PowerCyclesValid;
    }

    bool isNvme() const
    {
        return m_Nvme;
    }

    const SmartNvmeHealth &nvmeHealth() const
    {
        return m_NvmeHealth;
    }

    QList<SmartAttributeParsedData> attributes() const
    {
        return m_Attributes;    /**< @return a list that contains the disk SMART attributes */
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

    void setSectors(quint64 numSectors)
    {
        m_Sectors = numSectors;
    }

    void setTemperature(quint64 mkelvin)
    {
        m_Temperature = mkelvin;
        m_TemperatureValid = true;
    }

    void setPoweredOn(quint64 msec)
    {
        m_PoweredOn = msec;
        m_PoweredOnValid = true;
    }

    void setPowerCycles(quint64 powerCycleCt)
    {
        m_PowerCycles = powerCycleCt;
        m_PowerCyclesValid = true;
    }

    void setNvmeHealth(const SmartNvmeHealth &health)
    {
        m_Nvme = true;
        m_NvmeHealth = health;
    }

    void setSmartStatus(bool smartStatus)
    {
        m_SmartStatus = smartStatus;
    }

    void setSelfTestExecutionStatus(SmartStatus::SelfTestStatus status)
    {
        m_SelfTestExecutionStatus = status;
    }

    void addAttribute(SmartAttributeParsedData &attribute)
    {
        m_Attributes << attribute;
    }

protected:
    void validateBadAttributes();

    void updateNvmeOverall();

private:
    QString m_ModelName;
    QString m_FirmwareVersion;
    QString m_SerialNumber;
    quint64 m_Sectors;
    quint64 m_Temperature;
    quint64 m_BadSectors;
    quint64 m_PoweredOn;
    quint64 m_PowerCycles;
    bool m_TemperatureValid;
    bool m_PoweredOnValid;
    bool m_PowerCyclesValid;
    bool m_Nvme;
    bool m_SmartStatus;
    bool m_BadAttributeNow;
    bool m_BadAttributeInThePast;
    SmartStatus::SelfTestStatus m_SelfTestExecutionStatus;
    SmartStatus::Overall m_Overall;
    QList<SmartAttributeParsedData> m_Attributes;
    SmartNvmeHealth m_NvmeHealth;
};

#endif // SMARTDISKINFORMATION_H
