/*************************************************************************
 *  Copyright (C) 2010 by Volker Lanz <vl@fidra.de>                      *
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

#ifndef KPMCORE_SMARTSTATUS_H
#define KPMCORE_SMARTSTATUS_H

#include "util/libpartitionmanagerexport.h"
#include "core/smartattribute.h"

#include <QtGlobal>
#include <QString>
#include <QList>

struct SkSmartAttributeParsedData;
struct SkDisk;

class LIBKPMCORE_EXPORT SmartStatus
{
public:
    enum class Overall {
        Good,
        BadPast,
        BadSectors,
        BadNow,
        BadSectorsMany,
        Bad
    };

    enum class SelfTestStatus {
        Success,
        Aborted,
        Interrupted,
        Fatal,
        ErrorUnknown,
        ErrorEletrical,
        ErrorServo,
        ErrorRead,
        ErrorHandling,
        InProgress = 15,
    };

public:
    typedef QList<SmartAttribute> Attributes;

public:
    SmartStatus(const QString &device_path);

public:
    void update();

    const QString &devicePath() const
    {
        return m_DevicePath;
    }
    bool isValid() const
    {
        return m_InitSuccess;
    }
    bool status() const
    {
        return m_Status;
    }
    const QString &modelName() const
    {
        return m_ModelName;
    }
    const QString &serial() const
    {
        return m_Serial;
    }
    const QString &firmware() const
    {
        return m_Firmware;
    }
    quint64 temp() const
    {
        return m_Temp;
    }
    quint64 badSectors() const
    {
        return m_BadSectors;
    }
    quint64 powerCycles() const
    {
        return m_PowerCycles;
    }
    quint64 poweredOn() const
    {
        return m_PoweredOn;
    }
    const Attributes &attributes() const
    {
        return m_Attributes;
    }
    Overall overall() const
    {
        return m_Overall;
    }
    SelfTestStatus selfTestStatus() const
    {
        return m_SelfTestStatus;
    }

    void addAttributes(QList<SmartAttributeParsedData> attr);

    static QString tempToString(quint64 mkelvin);
    static QString overallAssessmentToString(Overall o);
    static QString selfTestStatusToString(SmartStatus::SelfTestStatus s);

private:
    void setStatus(bool s)
    {
        m_Status = s;
    }
    void setModelName(const QString &name)
    {
        m_ModelName = name;
    }
    void setSerial(const QString &s)
    {
        m_Serial = s;
    }
    void setFirmware(const QString &f)
    {
        m_Firmware = f;
    }
    void setTemp(quint64 t)
    {
        m_Temp = t;
    }
    void setInitSuccess(bool b)
    {
        m_InitSuccess = b;
    }
    void setBadSectors(quint64 s)
    {
        m_BadSectors = s;
    }
    void setPowerCycles(quint64 p)
    {
        m_PowerCycles = p;
    }
    void setPoweredOn(quint64 t)
    {
        m_PoweredOn = t;
    }
    void setOverall(Overall o)
    {
        m_Overall = o;
    }
    void setSelfTestStatus(SelfTestStatus s)
    {
        m_SelfTestStatus = s;
    }

private:
    const QString m_DevicePath;
    bool m_InitSuccess;
    bool m_Status;
    QString m_ModelName;
    QString m_Serial;
    QString m_Firmware;
    Overall m_Overall;
    SelfTestStatus m_SelfTestStatus;
    quint64 m_Temp;
    quint64 m_BadSectors;
    quint64 m_PowerCycles;
    quint64 m_PoweredOn;
    Attributes m_Attributes;
};

#endif
