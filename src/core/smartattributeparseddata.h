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

#ifndef KPMCORE_SMARTATTRIBUTEPARSEDDATA_H
#define KPMCORE_SMARTATTRIBUTEPARSEDDATA_H

#include <QJsonObject>
#include <QString>

class SmartDiskInformation;

/** SMART Quirk */
enum SmartQuirk {
    None                                    = 0x000000,
    SMART_QUIRK_9_POWERONMINUTES            = 0x000001,
    SMART_QUIRK_9_POWERONSECONDS            = 0x000002,
    SMART_QUIRK_9_POWERONHALFMINUTES        = 0x000004,
    SMART_QUIRK_192_EMERGENCYRETRACTCYCLECT = 0x000008,
    SMART_QUIRK_193_LOADUNLOAD              = 0x000010,
    SMART_QUIRK_194_10XCELSIUS              = 0x000020,
    SMART_QUIRK_194_UNKNOWN                 = 0x000040,
    SMART_QUIRK_200_WRITEERRORCOUNT         = 0x000080,
    SMART_QUIRK_201_DETECTEDTACOUNT         = 0x000100,
    SMART_QUIRK_5_UNKNOWN                   = 0x000200,
    SMART_QUIRK_9_UNKNOWN                   = 0x000400,
    SMART_QUIRK_197_UNKNOWN                 = 0x000800,
    SMART_QUIRK_198_UNKNOWN                 = 0x001000,
    SMART_QUIRK_190_UNKNOWN                 = 0x002000,
    SMART_QUIRK_232_AVAILABLERESERVEDSPACE  = 0x004000,
    SMART_QUIRK_233_MEDIAWEAROUTINDICATOR   = 0x008000,
    SMART_QUIRK_225_TOTALLBASWRITTEN        = 0x010000,
    SMART_QUIRK_4_UNUSED                    = 0x020000,
    SMART_QUIRK_226_TIMEWORKLOADMEDIAWEAR   = 0x040000,
    SMART_QUIRK_227_TIMEWORKLOADHOSTREADS   = 0x080000,
    SMART_QUIRK_228_WORKLOADTIMER           = 0x100000,
    SMART_QUIRK_3_UNUSED                    = 0x200000,
};

/** A unit for SMART attributes */
enum class SmartAttributeUnit {
    Unknown,
    None,
    Miliseconds,
    Sectors,
    Milikelvin,
    SmallPercent, /* percentage with 3 decimal points */
    Percent,
    MB,
};

/** A SMART parsed attribute.

    It receives the attribute data from JSON, retrieve its data and validates its values.

    @author Caio Carvalho <caiojcarvalho@gmail.com>
*/
class SmartAttributeParsedData
{
public:
    /** SMART Quirk to some particular model and firmware */
    struct SmartQuirkDataBase {
        QString model;
        QString firmware;
        SmartQuirk quirk;

        SmartQuirkDataBase(const QString &m = QString(),
                           const QString &f = QString(),
                           SmartQuirk q = SmartQuirk::None) :
            model(m),
            firmware(f),
            quirk(q)
        {
        };
    };

public:
    SmartAttributeParsedData(SmartDiskInformation *disk, QJsonObject jsonAttribute);

    SmartAttributeParsedData(const SmartAttributeParsedData &other);

public:
    quint32 id() const
    {
        return m_Id;    /**< @return attribute id */
    }

    qint32 currentValue() const
    {
        return m_CurrentValue;    /**< @return attribute current value */
    }

    qint32 worstValue() const
    {
        return m_WorstValue;    /**< @return attribute worst value */
    }

    qint32 threshold() const
    {
        return m_Threshold;    /**< @return attribute threshold value */
    }

    bool prefailure() const
    {
        return m_Prefailure;    /**< @return attribute prefailure status */
    }

    bool online() const
    {
        return m_Online;    /**< @return attribute online status */
    }

    quint64 raw() const
    {
        return m_Raw;    /**< @return attribute raw value */
    }

    quint64 prettyValue() const
    {
        return m_PrettyValue;    /**< @return attribute pretty value */
    }

    SmartAttributeUnit prettyUnit() const
    {
        return m_PrettyUnit;    /**< @return pretty unit value */
    }

    bool goodNowValid() const
    {
        return m_GoodNowValid;    /**< @return good now attribute status validation */
    }

    bool goodNow() const
    {
        return m_GoodNow;    /**< @return good now attribute status */
    }

    bool goodInThePastValid() const
    {
        return m_GoodInThePastValid;    /**< @return good in the past attribute status validation */
    }

    bool goodInThePast() const
    {
        return m_GoodInThePast;    /**< @return good in the past attribute status */
    }

    bool thresholdValid() const
    {
        return m_ThresholdValid;    /**< @return threshold value validation */
    }

    bool currentValueValid() const
    {
        return m_CurrentValueValid;    /**< @return current value validation */
    }

    bool worstValueValid() const
    {
        return m_WorstValueValid;    /**< @return worst value validation */
    }

    bool warn() const
    {
        return m_Warn;    /**< @return warn status */
    }

    SmartDiskInformation *disk() const
    {
        return m_Disk;    /**< @return attribute's disk reference */
    }

protected:
    void validateValues();

    bool updateUnit();

    void makePretty();

    void verifyAttribute();

    void verifyTemperature();

    void verifyShortTime();

    void verifyLongTime();

    void verifySectors();

private:
    quint32 m_Id;
    qint32 m_CurrentValue;
    qint32 m_WorstValue;
    qint32 m_Threshold;
    quint64 m_Raw;
    quint64 m_PrettyValue;
    bool m_CurrentValueValid;
    bool m_WorstValueValid;
    bool m_ThresholdValid;
    bool m_Prefailure;
    bool m_Online;
    bool m_GoodNow;
    bool m_GoodNowValid;
    bool m_GoodInThePast;
    bool m_GoodInThePastValid;
    bool m_Warn;
    SmartAttributeUnit m_PrettyUnit;
    SmartDiskInformation *m_Disk;
    SmartQuirk m_Quirk;
};

#endif // SMARTATTRIBUTEPARSEDDATA_H
