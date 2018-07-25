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

#include "smartattributeparseddata.h"
#include "core/smartdiskinformation.h"

#include <QJsonObject>
#include <QMap>
#include <QRegularExpression>
#include <QVariant>
#include <QVector>

#define MKELVIN_VALID_MIN ((qint64) ((-15LL*1000LL) + 273150LL))
#define MKELVIN_VALID_MAX ((qint64) ((100LL*1000LL) + 273150LL))

#define MSECOND_VALID_MIN 1ULL
#define MSECOND_VALID_SHORT_MAX (60ULL * 60ULL * 1000ULL)
#define MSECOND_VALID_LONG_MAX (30ULL * 365ULL * 24ULL * 60ULL * 60ULL * 1000ULL)

static QMap<qint32, SmartAttributeUnit> tableUnit();
static SmartQuirk getQuirk(QString model, QString firmware);

/** Creates a new SmartAttributeParsedData object.
    @param disk the reference to the disk that this attribute is allocated to
    @param jsonAttribute JSON attribute data
*/
SmartAttributeParsedData::SmartAttributeParsedData(SmartDiskInformation *disk,
                                                   QJsonObject jsonAttribute) :
    m_Id(0),
    m_CurrentValue(0),
    m_WorstValue(0),
    m_Threshold(0),
    m_Raw(0),
    m_PrettyValue(0),
    m_CurrentValueValid(false),
    m_WorstValueValid(false),
    m_ThresholdValid(false),
    m_Prefailure(false),
    m_Online(false),
    m_GoodNow(true),
    m_GoodNowValid(false),
    m_GoodInThePast(true),
    m_GoodInThePastValid(false),
    m_Warn(false),
    m_PrettyUnit(SmartAttributeUnit::Unknown),
    m_Disk(disk),
    m_Quirk(SmartQuirk::None)
{
    if (disk)
        m_Quirk = getQuirk(disk->model(), disk->firmware());

    if (!jsonAttribute.isEmpty()) {
        QString id = QStringLiteral("id");
        QString value = QStringLiteral("value");
        QString worst = QStringLiteral("worst");
        QString thresh = QStringLiteral("thresh");
        QString raw = QStringLiteral("raw");
        QString flags = QStringLiteral("flags");
        QString prefailure = QStringLiteral("prefailure");
        QString online = QStringLiteral("updated_online");

        m_Id = jsonAttribute[id].toInt();
        m_CurrentValue = jsonAttribute[value].toInt();
        m_WorstValue = jsonAttribute[worst].toInt();
        m_Threshold = jsonAttribute[thresh].toInt();

        QJsonObject rawObj = jsonAttribute[raw].toObject();

        m_Raw = rawObj[value].toVariant().toULongLong();

        QJsonObject flagsObj = jsonAttribute[flags].toObject();

        m_Prefailure = flagsObj[prefailure].toBool();
        m_Online = flagsObj[online].toBool();

        if (!updateUnit())
            m_PrettyUnit = SmartAttributeUnit::Unknown;

        makePretty();

        validateValues();

        verifyAttribute();
    }
}

/** @param other SmartAttributeParsedData to copy
*/
SmartAttributeParsedData::SmartAttributeParsedData(const SmartAttributeParsedData &other) :
    m_Id(other.id()),
    m_CurrentValue(other.currentValue()),
    m_WorstValue(other.worstValue()),
    m_Threshold(other.threshold()),
    m_Raw(other.raw()),
    m_PrettyValue(other.prettyValue()),
    m_CurrentValueValid(other.currentValueValid()),
    m_WorstValueValid(other.worstValueValid()),
    m_ThresholdValid(other.thresholdValid()),
    m_Prefailure(other.prefailure()),
    m_Online(other.online()),
    m_GoodNow(other.goodNow()),
    m_GoodNowValid(other.goodNowValid()),
    m_GoodInThePast(other.goodInThePast()),
    m_GoodInThePastValid(other.goodInThePastValid()),
    m_Warn(other.warn()),
    m_PrettyUnit(other.prettyUnit()),
    m_Disk(other.disk()),
    m_Quirk(other.m_Quirk)
{

}

/** Validate values from the current attribute */
void SmartAttributeParsedData::validateValues()
{
    m_CurrentValueValid = m_CurrentValue >= 1 && m_CurrentValue <= 0xFD;
    m_WorstValueValid = m_WorstValue >= 1 && m_WorstValue <= 0xFD;
    m_ThresholdValid = m_Threshold != 0xFE;

    if (m_Threshold >= 1 && m_Threshold <= 0xFD) {
        if (m_WorstValueValid) {
            m_GoodInThePast = m_GoodInThePast && (m_WorstValue > m_Threshold);
            m_GoodInThePastValid = true;
        }
        if (m_CurrentValueValid) {
            m_GoodNow = m_GoodNow && (m_CurrentValue > m_Threshold);
            m_GoodNowValid = true;
        }
    }

    m_Warn = (m_GoodNowValid && !m_GoodNow) || (m_GoodInThePastValid && !m_GoodInThePast);
}

/** Make a pretty value from raw based on attribute's id */
void SmartAttributeParsedData::makePretty()
{
    if (m_PrettyUnit == SmartAttributeUnit::Unknown)
        return;

    switch (id()) {
    case 3:
        m_PrettyValue = raw() & 0xFFFF;
        break;

    case 5:
        m_PrettyValue = raw() & 0xFFFFFFFFU;
        break;

    case 9:
        m_PrettyValue = (raw() & 0xFFFFFFFFU) * 60 * 60 * 1000;
        break;

    case 170:
        m_PrettyValue = currentValue();
        break;

    case 190:
        m_PrettyValue = (raw() & 0xFFFF) * 1000 + 273150;
        break;

    case 194:
        m_PrettyValue = (raw() & 0xFFFF) * 1000 + 273150;
        break;

    case 197:
        m_PrettyValue = (raw() & 0xFFFFFFFFU);
        break;

    case 222:
        m_PrettyValue = (raw() & 0xFFFFFFFFU) * 60 * 60 * 1000;
        break;

    case 228:
        m_PrettyValue = raw() * 60 * 1000;
        break;

    case 231:
        m_PrettyValue = (raw() & 0xFFFF) * 1000 + 273150;
        break;

    case 232:
        m_PrettyValue = currentValue();
        break;

    case 240:
        m_PrettyValue = (raw() & 0xFFFFFFFFU) * 60 * 60 * 1000;
        break;

    case 241:
        m_PrettyValue = raw() * 65536ULL * 512ULL / 1000000ULL;
        break;

    case 242:
        m_PrettyValue = raw() * 65536ULL * 512ULL / 1000000ULL;
        break;

    default:
        m_PrettyValue = raw();
        break;

    }
}

/** Verify attribute's unit */
void SmartAttributeParsedData::verifyAttribute()
{
    if (id() == 3 || id() == 226)
        verifyShortTime();
    else if (id() == 5 || id() == 187 || id() == 197 || id() == 198)
        verifySectors();
    else if (id() == 9 || id() == 222 || id() == 240)
        verifyLongTime();
    else if (id() == 190 || id() == 194 || id() == 231)
        verifyTemperature();
}

void SmartAttributeParsedData::verifyTemperature()
{
    if (prettyUnit() != SmartAttributeUnit::Milikelvin)
        return;

    if (prettyValue() < MKELVIN_VALID_MIN || prettyValue() > MKELVIN_VALID_MAX)
        m_PrettyUnit = SmartAttributeUnit::Unknown;
}

void SmartAttributeParsedData::verifyShortTime()
{
    if (prettyUnit() != SmartAttributeUnit::Miliseconds)
        return;

    if (prettyValue() < MSECOND_VALID_MIN || prettyValue() > MSECOND_VALID_SHORT_MAX)
        m_PrettyUnit = SmartAttributeUnit::Unknown;
}

void SmartAttributeParsedData::verifyLongTime()
{
    if (prettyUnit() != SmartAttributeUnit::Miliseconds)
        return;

    if (prettyValue() < MSECOND_VALID_MIN || prettyValue() > MSECOND_VALID_LONG_MAX)
        m_PrettyUnit = SmartAttributeUnit::Unknown;
}

void SmartAttributeParsedData::verifySectors()
{
    if (prettyUnit() != SmartAttributeUnit::Sectors)
        return;

    quint64 maxSectors = disk()->size() / 512ULL;

    if (prettyValue() == 0xFFFFFFFFULL || prettyValue() == 0xFFFFFFFFFFFFULL || (maxSectors > 0
                                                                                 && prettyValue() > maxSectors))
        m_PrettyUnit = SmartAttributeUnit::Unknown;
    else if ((id() == 5 || id() == 197) && prettyValue() > 0)
        m_Warn = true;
}

bool SmartAttributeParsedData::updateUnit()
{
    if (m_Quirk) {
        switch (id()) {
        case 3:
            if (m_Quirk & SmartQuirk::SMART_QUIRK_3_UNUSED) {
                m_PrettyUnit = SmartAttributeUnit::Unknown;
                return true;
            }
            break;

        case 4:
            if (m_Quirk & SmartQuirk::SMART_QUIRK_4_UNUSED) {
                m_PrettyUnit = SmartAttributeUnit::Unknown;
                return true;
            }
            break;

        case 5:
            if (m_Quirk & SmartQuirk::SMART_QUIRK_5_UNKNOWN)
                return false;
            break;

        case 9:
            if (m_Quirk & SmartQuirk::SMART_QUIRK_9_POWERONMINUTES) {
                m_PrettyUnit = SmartAttributeUnit::Miliseconds;
                return true;
            } else if (m_Quirk & SmartQuirk::SMART_QUIRK_9_POWERONSECONDS) {
                m_PrettyUnit = SmartAttributeUnit::Miliseconds;
                return true;
            } else if (m_Quirk & SmartQuirk::SMART_QUIRK_9_POWERONHALFMINUTES) {
                m_PrettyUnit = SmartAttributeUnit::Miliseconds;
                return true;
            } else if (m_Quirk & SmartQuirk::SMART_QUIRK_9_UNKNOWN)
                return false;
            break;

        case 190:
            if (m_Quirk & SmartQuirk::SMART_QUIRK_190_UNKNOWN)
                return false;
            break;

        case 192:
            if (m_Quirk & SmartQuirk::SMART_QUIRK_192_EMERGENCYRETRACTCYCLECT) {
                m_PrettyUnit = SmartAttributeUnit::None;
                return true;
            }
            break;

        case 194:
            if (m_Quirk & SmartQuirk::SMART_QUIRK_194_10XCELSIUS) {
                m_PrettyUnit = SmartAttributeUnit::Milikelvin;
                return true;
            } else if (m_Quirk & SmartQuirk::SMART_QUIRK_194_UNKNOWN)
                return false;
            break;

        case 197:
            if (m_Quirk & SmartQuirk::SMART_QUIRK_197_UNKNOWN)
                return false;
            break;

        case 198:
            if (m_Quirk & SmartQuirk::SMART_QUIRK_198_UNKNOWN)
                return false;
            break;

        case 200:
            if (m_Quirk & SmartQuirk::SMART_QUIRK_200_WRITEERRORCOUNT) {
                m_PrettyUnit = SmartAttributeUnit::None;
                return true;
            }
            break;

        case 201:
            if (m_Quirk & SmartQuirk::SMART_QUIRK_201_DETECTEDTACOUNT) {
                m_PrettyUnit = SmartAttributeUnit::None;
                return true;
            }
            break;

        case 225:
            if (m_Quirk & SmartQuirk::SMART_QUIRK_225_TOTALLBASWRITTEN) {
                m_PrettyUnit = SmartAttributeUnit::MB;
                return true;
            }
            break;

        case 226:
            if (m_Quirk & SmartQuirk::SMART_QUIRK_226_TIMEWORKLOADMEDIAWEAR) {
                m_PrettyUnit = SmartAttributeUnit::SmallPercent;
                return true;
            }
            break;

        case 227:
            if (m_Quirk & SmartQuirk::SMART_QUIRK_227_TIMEWORKLOADHOSTREADS) {
                m_PrettyUnit = SmartAttributeUnit::SmallPercent;
                return true;
            }
            break;

        case 228:
            if (m_Quirk & SmartQuirk::SMART_QUIRK_228_WORKLOADTIMER) {
                m_PrettyUnit = SmartAttributeUnit::Miliseconds;
                return true;
            }
            break;

        case 232:
            if (m_Quirk & SmartQuirk::SMART_QUIRK_232_AVAILABLERESERVEDSPACE) {
                m_PrettyUnit = SmartAttributeUnit::Percent;
                return true;
            }
            break;

        case 233:
            if (m_Quirk & SmartQuirk::SMART_QUIRK_233_MEDIAWEAROUTINDICATOR) {
                m_PrettyUnit = SmartAttributeUnit::Percent;
                return true;
            }
            break;

        }
    }

    if (tableUnit().contains(id())) {
        m_PrettyUnit = tableUnit()[id()];
        return true;
    }

    return false;
}

static QMap<qint32, SmartAttributeUnit> tableUnit()
{
    QMap<qint32, SmartAttributeUnit> table;
    table.insert(1, SmartAttributeUnit::None);
    table.insert(2, SmartAttributeUnit::Unknown);
    table.insert(3, SmartAttributeUnit::Miliseconds);
    table.insert(4, SmartAttributeUnit::None);
    table.insert(5, SmartAttributeUnit::Sectors);
    table.insert(6, SmartAttributeUnit::Unknown);
    table.insert(7, SmartAttributeUnit::None);
    table.insert(8, SmartAttributeUnit::Unknown);
    table.insert(9, SmartAttributeUnit::Miliseconds);
    table.insert(10, SmartAttributeUnit::None);
    table.insert(11, SmartAttributeUnit::None);
    table.insert(12, SmartAttributeUnit::None);
    table.insert(13, SmartAttributeUnit::None);
    table.insert(170, SmartAttributeUnit::Percent);
    table.insert(171, SmartAttributeUnit::None);
    table.insert(172, SmartAttributeUnit::None);
    table.insert(175, SmartAttributeUnit::None);
    table.insert(176, SmartAttributeUnit::None);
    table.insert(177, SmartAttributeUnit::None);
    table.insert(178, SmartAttributeUnit::None);
    table.insert(179, SmartAttributeUnit::None);
    table.insert(180, SmartAttributeUnit::None);
    table.insert(181, SmartAttributeUnit::None);
    table.insert(182, SmartAttributeUnit::None);
    table.insert(183, SmartAttributeUnit::None);
    table.insert(184, SmartAttributeUnit::None);
    table.insert(187, SmartAttributeUnit::Sectors);
    table.insert(188, SmartAttributeUnit::None);
    table.insert(189, SmartAttributeUnit::None);
    table.insert(190, SmartAttributeUnit::Milikelvin);
    table.insert(191, SmartAttributeUnit::None);
    table.insert(192, SmartAttributeUnit::None);
    table.insert(193, SmartAttributeUnit::None);
    table.insert(194, SmartAttributeUnit::Milikelvin);
    table.insert(195, SmartAttributeUnit::None);
    table.insert(196, SmartAttributeUnit::None);
    table.insert(197, SmartAttributeUnit::Sectors);
    table.insert(198, SmartAttributeUnit::Sectors);
    table.insert(199, SmartAttributeUnit::None);
    table.insert(200, SmartAttributeUnit::None);
    table.insert(201, SmartAttributeUnit::None);
    table.insert(202, SmartAttributeUnit::None);
    table.insert(203, SmartAttributeUnit::Unknown);
    table.insert(204, SmartAttributeUnit::None);
    table.insert(205, SmartAttributeUnit::None);
    table.insert(206, SmartAttributeUnit::Unknown);
    table.insert(207, SmartAttributeUnit::Unknown);
    table.insert(208, SmartAttributeUnit::Unknown);
    table.insert(209, SmartAttributeUnit::Unknown);
    table.insert(220, SmartAttributeUnit::Unknown);
    table.insert(221, SmartAttributeUnit::None);
    table.insert(222, SmartAttributeUnit::Miliseconds);
    table.insert(223, SmartAttributeUnit::None);
    table.insert(224, SmartAttributeUnit::Unknown);
    table.insert(225, SmartAttributeUnit::None);
    table.insert(226, SmartAttributeUnit::Miliseconds);
    table.insert(227, SmartAttributeUnit::None);
    table.insert(228, SmartAttributeUnit::None);
    table.insert(230, SmartAttributeUnit::Unknown);
    table.insert(231, SmartAttributeUnit::Milikelvin);
    table.insert(232, SmartAttributeUnit::Percent);
    table.insert(233, SmartAttributeUnit::Unknown);
    table.insert(234, SmartAttributeUnit::Sectors);
    table.insert(235, SmartAttributeUnit::Unknown);
    table.insert(240, SmartAttributeUnit::Miliseconds);
    table.insert(241, SmartAttributeUnit::MB);
    table.insert(242, SmartAttributeUnit::MB);
    table.insert(250, SmartAttributeUnit::None);

    return table;
}

static const QVector<SmartAttributeParsedData::SmartQuirkDataBase> quirkDatabase()
{
    typedef SmartAttributeParsedData::SmartQuirkDataBase QuirkDatabase;

    QVector<QuirkDatabase> quirkDb;

    quirkDb << QuirkDatabase(QStringLiteral("^(FUJITSU MHY2120BH|FUJITSU MHY2250BH)$"), QStringLiteral("^0085000B$"),
                            static_cast<SmartQuirk>(SmartQuirk::SMART_QUIRK_9_POWERONMINUTES |
                                                    SmartQuirk::SMART_QUIRK_197_UNKNOWN |
                                                    SmartQuirk::SMART_QUIRK_198_UNKNOWN));

    quirkDb << QuirkDatabase(QStringLiteral("^FUJITSU MHR2040AT$"), QStringLiteral(), 
                            static_cast<SmartQuirk>(SmartQuirk::SMART_QUIRK_9_POWERONSECONDS |
                                                    SmartQuirk::SMART_QUIRK_192_EMERGENCYRETRACTCYCLECT |
                                                    SmartQuirk::SMART_QUIRK_200_WRITEERRORCOUNT));

    quirkDb << QuirkDatabase(QStringLiteral("^FUJITSU MHS20[6432]0AT(  .)?$"), QStringLiteral(),
                            static_cast<SmartQuirk>(SmartQuirk::SMART_QUIRK_9_POWERONSECONDS |
                                                    SmartQuirk::SMART_QUIRK_192_EMERGENCYRETRACTCYCLECT |
                                                    SmartQuirk::SMART_QUIRK_200_WRITEERRORCOUNT |
                                                    SmartQuirk::SMART_QUIRK_201_DETECTEDTACOUNT));

    quirkDb << QuirkDatabase(QStringLiteral("^("
                            "FUJITSU M1623TAU|"
                            "FUJITSU MHG2...ATU?.*|"
                            "FUJITSU MHH2...ATU?.*|"
                            "FUJITSU MHJ2...ATU?.*|"
                            "FUJITSU MHK2...ATU?.*|"
                            "FUJITSU MHL2300AT|"
                            "FUJITSU MHM2(20|15|10|06)0AT|"
                            "FUJITSU MHN2...AT|"
                            "FUJITSU MHR2020AT|"
                            "FUJITSU MHT2...(AH|AS|AT|BH)U?.*|"
                            "FUJITSU MHU2...ATU?.*|"
                            "FUJITSU MHV2...(AH|AS|AT|BH|BS|BT).*|"
                            "FUJITSU MP[A-G]3...A[HTEV]U?.*"
                            ")$"),
                            QStringLiteral(),
                            SmartQuirk::SMART_QUIRK_9_POWERONSECONDS);

    quirkDb << QuirkDatabase(QStringLiteral("^("
                            "SAMSUNG SV4012H|"
                            "SAMSUNG SP(0451|08[0124]2|12[0145]3|16[0145]4)[CN]"
                            ")$"),
                            QStringLiteral(),
                            SmartQuirk::SMART_QUIRK_9_POWERONHALFMINUTES);

    quirkDb << QuirkDatabase(QStringLiteral("^("
                            "SAMSUNG SV0412H|"
                            "SAMSUNG SV1204H"
                            ")$"),
                            QStringLiteral(),
                            static_cast<SmartQuirk>(SmartQuirk::SMART_QUIRK_9_POWERONHALFMINUTES | SmartQuirk::SMART_QUIRK_194_10XCELSIUS));

    quirkDb << QuirkDatabase(QStringLiteral("^SAMSUNG SP40A2H$"),
                            QStringLiteral("^RR100-07$"),
                            SmartQuirk::SMART_QUIRK_9_POWERONHALFMINUTES);

    quirkDb << QuirkDatabase(QStringLiteral("^SAMSUNG SP80A4H$"),
                            QStringLiteral("^RT100-06$"),
                            SmartQuirk::SMART_QUIRK_9_POWERONHALFMINUTES);

    quirkDb << QuirkDatabase(QStringLiteral("^SAMSUNG SP8004H$"),
                            QStringLiteral("^QW100-61$"),
                            SmartQuirk::SMART_QUIRK_9_POWERONHALFMINUTES);

    quirkDb << QuirkDatabase(QStringLiteral("^("
                            "Maxtor 2B0(0[468]|1[05]|20)H1|"
                            "Maxtor 4G(120J6|160J[68])|"
                            "Maxtor 4D0(20H1|40H2|60H3|80H4)"
                            ")$"),
                            QStringLiteral(),
                            static_cast<SmartQuirk>(SmartQuirk::SMART_QUIRK_9_POWERONMINUTES | SmartQuirk::SMART_QUIRK_194_UNKNOWN));

    quirkDb << QuirkDatabase(QStringLiteral("^("
                            "Maxtor 2F0[234]0[JL]0|"
                            "Maxtor 8(1280A2|2160A4|2560A4|3840A6|4000A6|5120A8)|"
                            "Maxtor 8(2160D2|3228D3|3240D3|4320D4|6480D6|8400D8|8455D8)|"
                            "Maxtor 9(0510D4|0576D4|0648D5|0720D5|0840D6|0845D6|0864D6|1008D7|1080D8|1152D8)|"
                            "Maxtor 9(1(360|350|202)D8|1190D7|10[12]0D6|0840D5|06[48]0D4|0510D3|1(350|202)E8|1010E6|0840E5|0640E4)|"
                            "Maxtor 9(0512D2|0680D3|0750D3|0913D4|1024D4|1360D6|1536D6|1792D7|2048D8)|"
                            "Maxtor 9(2732U8|2390U7|204[09]U6|1707U5|1366U4|1024U3|0845U3|0683U2)|"
                            "Maxtor 4(R0[68]0[JL]0|R1[26]0L0|A160J0|R120L4)|"
                            "Maxtor (91728D8|91512D7|91303D6|91080D5|90845D4|90645D3|90648D[34]|90432D2)|"
                            "Maxtor 9(0431U1|0641U2|0871U2|1301U3|1741U4)|"
                            "Maxtor (94091U8|93071U6|92561U5|92041U4|91731U4|91531U3|91361U3|91021U2|90841U2|90651U2)|"
                            "Maxtor (33073U4|32049U3|31536U2|30768U1|33073H4|32305H3|31536H2|30768H1)|"
                            "Maxtor (93652U8|92739U6|91826U4|91369U3|90913U2|90845U2|90435U1)|"
                            "Maxtor 9(0684U2|1024U2|1362U3|1536U3|2049U4|2562U5|3073U6|4098U8)|"
                            "Maxtor (54098[UH]8|53073[UH]6|52732[UH]6|52049[UH]4|51536[UH]3|51369[UH]3|51024[UH]2)|"
                            "Maxtor 3(1024H1|1535H2|2049H2|3073H3|4098H4)( B)?|"
                            "Maxtor 5(4610H6|4098H6|3073H4|2049H3|1536H2|1369H2|1023H2)|"
                            "Maxtor 9(1023U2|1536U2|2049U3|2305U3|3073U4|4610U6|6147U8)|"
                            "Maxtor 9(1023H2|1536H2|2049H3|2305H3|3073H4|4098H6|4610H6|6147H8)|"
                            "Maxtor 5T0(60H6|40H4|30H3|20H2|10H1)|"
                            "Maxtor (98196H8|96147H6)|"
                            "Maxtor 4W(100H6|080H6|060H4|040H3|030H2)|"
                            "Maxtor 6(E0[234]|K04)0L0|"
                            "Maxtor 6(B(30|25|20|16|12|10|08)0[MPRS]|L(080[MLP]|(100|120)[MP]|160[MP]|200[MPRS]|250[RS]|300[RS]))0|"
                            "Maxtor 6Y((060|080|120|160)L0|(060|080|120|160|200|250)P0|(060|080|120|160|200|250)M0)|"
                            "Maxtor 7Y250[PM]0|"
                            "Maxtor [45]A(25|30|32)0[JN]0|"
                            "Maxtor 7L(25|30)0[SR]0"
                            ")$"),
                            QStringLiteral(),
                            SmartQuirk::SMART_QUIRK_9_POWERONMINUTES);

    quirkDb << QuirkDatabase(QStringLiteral("^("
                            "HITACHI_DK14FA-20B|"
                            "HITACHI_DK23..-..B?|"
                            "HITACHI_DK23FA-20J|HTA422020F9AT[JN]0|"
                            "HE[JN]4230[23]0F9AT00|"
                            "HTC4260[23]0G5CE00|HTC4260[56]0G8CE00"
                            ")$"),
                            QStringLiteral(),
                            static_cast<SmartQuirk>(SmartQuirk::SMART_QUIRK_9_POWERONMINUTES | SmartQuirk::SMART_QUIRK_193_LOADUNLOAD));

    quirkDb << QuirkDatabase(QStringLiteral("^HTS541010G9SA00$"),
                            QStringLiteral("^MBZOC60P$"),
                            SmartQuirk::SMART_QUIRK_5_UNKNOWN);

    quirkDb << QuirkDatabase(QStringLiteral("^MCCOE64GEMPP$"),
                            QStringLiteral("^2.9.0[3-9]$"),
                            static_cast<SmartQuirk>(SmartQuirk::SMART_QUIRK_5_UNKNOWN | SmartQuirk::SMART_QUIRK_190_UNKNOWN));

    quirkDb << QuirkDatabase(QStringLiteral("^INTEL SSDSA2(CT|BT|CW|BW)[0-9]{3}G3.*$"), 
                            QStringLiteral(), 
                            static_cast<SmartQuirk>(SmartQuirk::SMART_QUIRK_3_UNUSED |
                                                    SmartQuirk::SMART_QUIRK_4_UNUSED |
                                                    SmartQuirk::SMART_QUIRK_225_TOTALLBASWRITTEN |
                                                    SmartQuirk::SMART_QUIRK_226_TIMEWORKLOADMEDIAWEAR |
                                                    SmartQuirk::SMART_QUIRK_227_TIMEWORKLOADHOSTREADS |
                                                    SmartQuirk::SMART_QUIRK_228_WORKLOADTIMER |
                                                    SmartQuirk::SMART_QUIRK_232_AVAILABLERESERVEDSPACE |
                                                    SmartQuirk::SMART_QUIRK_233_MEDIAWEAROUTINDICATOR));

    return quirkDb;
}

static SmartQuirk getQuirk(QString model, QString firmware)
{
    const QVector<SmartAttributeParsedData::SmartQuirkDataBase> db = quirkDatabase();

    QRegularExpression modelRegex;
    QRegularExpression firmwareRegex;

    for (const SmartAttributeParsedData::SmartQuirkDataBase &item : qAsConst(db)) {
        if (!item.model.isEmpty()) {
            modelRegex.setPattern(item.model);
            QRegularExpressionMatch match = modelRegex.match(model);
            if (!match.hasMatch())
                continue;
        }
        if (!item.firmware.isEmpty()) {
            firmwareRegex.setPattern(item.firmware);
            QRegularExpressionMatch match = firmwareRegex.match(firmware);
            if (!match.hasMatch())
                continue;
        }
        return item.quirk;
    }

    return SmartQuirk::None;
}
