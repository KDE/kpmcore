/*
    SPDX-FileCopyrightText: 2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2016-2018 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2018 Caio Jordão Carvalho <caiojcarvalho@gmail.com>
    SPDX-FileCopyrightText: 2018 Anthony Fieroni <bvbfan@abv.bg>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "core/smartattribute.h"
#include "core/smartstatus.h"
#include "core/smartattributeparseddata.h"

#include <QLocale>

#include <KLocalizedString>
#include <KFormat>

static QString getAttrName(qint32 id);
static QString getAttrDescription(qint32 id);
static QString getPrettyValue(quint64 value, SmartAttributeUnit unit);
static SmartAttribute::Assessment getAssessment(const SmartAttributeParsedData& a);
static QString getRaw(quint64 raw);

SmartAttribute::SmartAttribute(const SmartAttributeParsedData& a) :
    m_Id(a.id()),
    m_Name(getAttrName(a.id())),
    m_Desc(getAttrDescription(a.id())),
    m_FailureType(a.prefailure() ? FailureType::PreFailure : FailureType::OldAge),
    m_UpdateType(a.online() ? UpdateType::Online : UpdateType::Offline),
    m_Current(a.currentValueValid() ?  a.currentValue() : -1),
    m_Worst(a.worstValueValid() ? a.worstValue() : -1),
    m_Threshold(a.thresholdValid() ? a.threshold() : -1),
    m_Raw(getRaw(a.raw())),
    m_Assessment(getAssessment(a)),
    m_Value(getPrettyValue(a.prettyValue(), a.prettyUnit()))
{

}

QString SmartAttribute::assessmentToString(Assessment a)
{
    switch (a) {
    case Assessment::Failing:
        return xi18nc("@item:intable", "failing");

    case Assessment::HasFailed:
        return xi18nc("@item:intable", "has failed");

    case Assessment::Warning:
        return xi18nc("@item:intable", "warning");

    case Assessment::Good:
        return xi18nc("@item:intable", "good");

    case Assessment::NotApplicable:
    default:
        return xi18nc("@item:intable not applicable", "N/A");
    }
}

static QString getPrettyValue(quint64 value, SmartAttributeUnit unit)
{
    QString rval;

    switch (unit) {
    case SmartAttributeUnit::Miliseconds:
        rval = KFormat().formatSpelloutDuration(value);
        break;

    case SmartAttributeUnit::Sectors:
        rval = xi18ncp("@item:intable", "%1 sector", "%1 sectors", value);
        break;

    case SmartAttributeUnit::Milikelvin:
        rval = SmartStatus::tempToString(value);
        break;

    case SmartAttributeUnit::None:
        rval = QLocale().toString(value);
        break;

    case SmartAttributeUnit::Unknown:
    default:
        rval = xi18nc("@item:intable not applicable", "N/A");
        break;
    }

    return rval;
}

typedef struct {
    qint32 id;
    const QString name;
    const QString desc;
} AttrDetails;

static const AttrDetails* attrDetails()
{
    static const AttrDetails details[] = {
        { 1,  i18nc("SMART attr name", "Read Error Rate"), i18nc("SMART attr description", "Rate of hardware read errors while reading data from the disk surface.")  },
        { 2,  i18nc("SMART attr name", "Throughput Performance"), i18nc("SMART attr description", "Overall (general) throughput performance of a hard disk drive. If the value of this attribute is decreasing there is a high probability that there is a problem with the disk.")  },
        { 3,  i18nc("SMART attr name", "Spin-Up Time"), i18nc("SMART attr description", "Average time of spindle spin up from zero RPM to fully operational.")  },
        { 4,  i18nc("SMART attr name", "Start/Stop Count"), i18nc("SMART attr description", "A tally of spindle start/stop cycles.")  },
        { 5,  i18nc("SMART attr name", "Reallocated Sectors Count"), i18nc("SMART attr description", "Count of reallocated sectors. When the hard drive finds a read/write/verification error, it marks this sector as &quot;reallocated&quot; and transfers data to a special reserved area (spare area).")  },
        { 6,  i18nc("SMART attr name", "Read Channel Margin"), i18nc("SMART attr description", "Margin of a channel while reading data. The function of this attribute is not specified.")  },
        { 7,  i18nc("SMART attr name", "Seek Error Rate"), i18nc("SMART attr description", "Rate of seek errors of the magnetic heads. If there is a partial failure in the mechanical positioning system, then seek errors will arise.")  },
        { 8,  i18nc("SMART attr name", "Seek Time Performance"), i18nc("SMART attr description", "Average performance of seek operations of the magnetic heads. If this attribute is decreasing, it is a sign of problems in the mechanical subsystem.")  },
        { 9,  i18nc("SMART attr name", "Power-On Hours"), i18nc("SMART attr description", "Count of hours in power-on state.")  },
        { 10,  i18nc("SMART attr name", "Spin Retry Count"), i18nc("SMART attr description", "Count of retry of spin start attempts if the first attempt was unsuccessful. An increase of this attribute value is a sign of problems in the hard disk mechanical subsystem.")  },
        { 11,  i18nc("SMART attr name", "Recalibration Retries"), i18nc("SMART attr description", "Count of recalibrations requested if the first attempt was unsuccessful. An increase of this attribute value is a sign of problems in the hard disk mechanical subsystem.")  },
        { 12,  i18nc("SMART attr name", "Power Cycle Count"), i18nc("SMART attr description", "Count of full hard disk power on/off cycles.")  },
        { 13,  i18nc("SMART attr name", "Soft Read Error Rate"), i18nc("SMART attr description", "Uncorrected read errors reported to the operating system.")  },
        { 170,  i18nc("SMART attr name", "SSD Available Reserved Space"), i18nc("SMART attr description", "Number of available reserved space as a percentage of reserved space.")  },
        { 171,  i18nc("SMART attr name", "SSD Program Fail Count"), i18nc("SMART attr description", "Number of flash program operation failures since the drive was deployed.") },
        { 172,  i18nc("SMART attr name", "SSD Erase Fail Count"), i18nc("SMART attr description", "Number of flash erase operation failures since the drive was deployed.") },
        { 173,  i18nc("SMART attr name", "SSD Wear Leveling Count"), i18nc("SMART attr description", "Counts the maximum worst erase count on any block.") },
        { 174,  i18nc("SMART attr name", "SSD Unexpected power loss count"), i18nc("SMART attr description", "Number of shutdowns without STANDBY IMMEDIATE as the last command (regardless of PLI activity using capacitor power). Normalized value is always 100.") },
        { 175,  i18nc("SMART attr name", "SSD Power Loss Protection Failure"), i18nc("SMART attr description", "Last test result, saturated at its maximum value. Bytes 0-1: last test result as microseconds to discharge cap in range [25, 5000000], lower indicates specific error code. Bytes 2-3: minutes since last test. Bytes 4-5: lifetime number of tests. Normalized value is set to 1 on test failure or 11 if the capacitor has been tested in an excessive temperature condition, otherwise 100.") },
        { 176,  i18nc("SMART attr name", "SSD Erase Fail Count (chip)"), i18nc("SMART attr description", "Number of flash erase command failures.") },
        { 177,  i18nc("SMART attr name", "SSD Wear Range Delta"), i18nc("SMART attr description", "Delta between most-worn and least-worn flash blocks.") },
        { 178,  i18nc("SMART attr name", "SSD Used Reserved Block Count Total"), i18nc("SMART attr description", "\"Pre-Fail\" Samsung attribute.") },
        { 179,  i18nc("SMART attr name", "SSD Used Reserved Block Count Total"), i18nc("SMART attr description", "\"Pre-Fail\" Samsung attribute.") },
        { 180,  i18nc("SMART attr name", "SSD Unused Reserved Block Count Total"), i18nc("SMART attr description", "\"Pre-Fail\" HP attribute.") },
        { 181,  i18nc("SMART attr name", "SSD Program Fail Count Total or Non-4K Aligned Access Count"), i18nc("SMART attr description", "Number of flash program operation failures since the drive was deployed.") },
        { 182,  i18nc("SMART attr name", "SSD Erase Fail Count"), i18nc("SMART attr description", "\"Pre-Fail\" Samsung attribute.") },
        { 183,  i18nc("SMART attr name", "SATA Downshift Error Count"), i18nc("SMART attr description", "Western Digital and Samsung attribute.")  },
        { 184,  i18nc("SMART attr name", "End-to-End Error"), i18nc("SMART attr description", "Part of HP's SMART IV technology: After transferring through the cache RAM data buffer the parity data between the host and the hard drive did not match.")  },
        { 185,  i18nc("SMART attr name", "Head Stability"), i18nc("SMART attr description", "Western Digital attribute.")  },
        { 186,  i18nc("SMART attr name", "Induced Op-Vibration Detection"), i18nc("SMART attr description", "Western Digital attribute.")  },
        { 187,  i18nc("SMART attr name", "Reported Uncorrectable Errors"), i18nc("SMART attr description", "Count of errors that could not be recovered using hardware ECC.")  },
        { 188,  i18nc("SMART attr name", "Command Timeout"), i18nc("SMART attr description", "Count of aborted operations due to HDD timeout.")  },
        { 189,  i18nc("SMART attr name", "High Fly Writes"), i18nc("SMART attr description", "Count of fly height errors detected.")  },
        { 190,  i18nc("SMART attr name", "Temperature Difference From 100"), i18nc("SMART attr description", "Value is equal to (100 &ndash; temp. °C), allowing manufacturer to set a minimum threshold which corresponds to a maximum temperature.")  },
        { 191,  i18nc("SMART attr name", "G-sense Error Rate"), i18nc("SMART attr description", "Count of errors resulting from externally-induced shock and vibration.")  },
        { 192,  i18nc("SMART attr name", "Power Off Retract Count"), i18nc("SMART attr description", "Count of power-off or emergency retract cycles")  },
        { 193,  i18nc("SMART attr name", "Load Cycle Count"), i18nc("SMART attr description", "Count of load/unload cycles into head landing zone position.")  },
        { 194,  i18nc("SMART attr name", "Temperature"), i18nc("SMART attr description", "Current internal temperature.")  },
        { 195,  i18nc("SMART attr name", "Hardware ECC Recovered"), i18nc("SMART attr description", "Count of errors that could be recovered using hardware ECC.")  },
        { 196,  i18nc("SMART attr name", "Reallocation Event Count"), i18nc("SMART attr description", "Count of remap operations. The raw value of this attribute shows the total number of attempts to transfer data from reallocated sectors to a spare area.")  },
        { 197,  i18nc("SMART attr name", "Current Pending Sector Count"), i18nc("SMART attr description", "Number of &quot;unstable&quot; sectors (waiting to be remapped, because of read errors).")  },
        { 198,  i18nc("SMART attr name", "Uncorrectable Sector Count"), i18nc("SMART attr description", "Count of uncorrectable errors when reading/writing a sector.")  },
        { 199,  i18nc("SMART attr name", "UltraDMA CRC Error Count"), i18nc("SMART attr description", "Count of errors in data transfer via the interface cable as determined by ICRC.")  },
        { 200,  i18nc("SMART attr name", "Multi-Zone Error Rate<br/>Write Error Rate"), i18nc("SMART attr description", "The total number of errors when writing a sector.")  },
        { 201,  i18nc("SMART attr name", "Soft Read Error Rate"), i18nc("SMART attr description", "Number of off-track errors.")  },
        { 202,  i18nc("SMART attr name", "Data Address Mark Errors"), i18nc("SMART attr description", "Number of Data Address Mark errors (or vendor-specific).")  },
        { 203,  i18nc("SMART attr name", "Run Out Cancel"), i18nc("SMART attr description", "Number of ECC errors")  },
        { 204,  i18nc("SMART attr name", "Soft ECC Correction"), i18nc("SMART attr description", "Number of errors corrected by software ECC")  },
        { 205,  i18nc("SMART attr name", "Thermal Asperity Rate"), i18nc("SMART attr description", "Number of errors due to high temperature.")  },
        { 206,  i18nc("SMART attr name", "Flying Height"), i18nc("SMART attr description", "Height of heads above the disk surface. A flying height that is too low increases the chances of a head crash while a flying height that is too high increases the chances of a read/write error.")  },
        { 207,  i18nc("SMART attr name", "Spin High Current"), i18nc("SMART attr description", "Amount of surge current used to spin up the drive.")  },
        { 208,  i18nc("SMART attr name", "Spin Buzz"), i18nc("SMART attr description", "Number of buzz routines needed to spin up the drive due to insufficient power.")  },
        { 209,  i18nc("SMART attr name", "Offline Seek Performance"), i18nc("SMART attr description", "Drive's seek performance during its internal tests.")  },
        { 211,  i18nc("SMART attr name", "Vibration During Write"), i18nc("SMART attr description", "Vibration During Write")  },
        { 212,  i18nc("SMART attr name", "Shock During Write"), i18nc("SMART attr description", "Shock During Write")  },
        { 220,  i18nc("SMART attr name", "Disk Shift"), i18nc("SMART attr description", "Distance the disk has shifted relative to the spindle (usually due to shock or temperature).")  },
        { 221,  i18nc("SMART attr name", "G-Sense Error Rate"), i18nc("SMART attr description", "The number of errors resulting from externally-induced shock and vibration.")  },
        { 222,  i18nc("SMART attr name", "Loaded Hours"), i18nc("SMART attr description", "Time spent operating under data load.")  },
        { 223,  i18nc("SMART attr name", "Load/Unload Retry Count"), i18nc("SMART attr description", "Number of times head changes position.")  },
        { 224,  i18nc("SMART attr name", "Load Friction"), i18nc("SMART attr description", "Resistance caused by friction in mechanical parts while operating.")  },
        { 225,  i18nc("SMART attr name", "Load/Unload Cycle Count"), i18nc("SMART attr description", "Total number of load cycles.")  },
        { 226,  i18nc("SMART attr name", "Load-In Time"), i18nc("SMART attr description", "Total time of loading on the magnetic heads actuator (time not spent in parking area).")  },
        { 227,  i18nc("SMART attr name", "Torque Amplification Count"), i18nc("SMART attr description", "Number of attempts to compensate for platter speed variations.")  },
        { 228,  i18nc("SMART attr name", "Power-Off Retract Cycle"), i18nc("SMART attr description", "The number of times the magnetic armature was retracted automatically as a result of cutting power.")  },
        { 230,  i18nc("SMART attr name", "GMR Head Amplitude"), i18nc("SMART attr description", "Amplitude of &quot;thrashing&quot; (distance of repetitive forward/reverse head motion)")  },
        { 231,  i18nc("SMART attr name", "Temperature"), i18nc("SMART attr description", "Drive Temperature")  },
        { 232,  i18nc("SMART attr name", "Endurance Remaining"), i18nc("SMART attr description", "Count of physical erase cycles completed on the drive as a percentage of the maximum physical erase cycles the drive supports")  },
        { 233,  i18nc("SMART attr name", "Power-On Seconds"), i18nc("SMART attr description", "Time elapsed in the power-on state")  },
        { 234,  i18nc("SMART attr name", "Unrecoverable ECC Count"), i18nc("SMART attr description", "Count of unrecoverable ECC errors")  },
        { 235,  i18nc("SMART attr name", "Good Block Rate"), i18nc("SMART attr description", "Count of available reserved blocks as percentage of the total number of reserved blocks")  },
        { 240,  i18nc("SMART attr name", "Head Flying Hours<br/>or Transfer Error Rate (Fujitsu)"), i18nc("SMART attr description", "Time while head is positioning<br/>or counts the number of times the link is reset during a data transfer.")  },
        { 241,  i18nc("SMART attr name", "Total LBAs Written"), i18nc("SMART attr description", "Total LBAs Written")  },
        { 242,  i18nc("SMART attr name", "Total LBAs Read"), i18nc("SMART attr description", "Total LBAs Read")  },
        { 249,  i18nc("SMART attr name", "SSD NAND_Writes_1GiB"), i18nc("SMART attr description", "Number of writes to NAND in 1 GB increments")  },
        { 250,  i18nc("SMART attr name", "Read Error Retry Rate"), i18nc("SMART attr description", "Number of errors while reading from a disk")  },
        { 254,  i18nc("SMART attr name", "Free Fall Protection"), i18nc("SMART attr description", "Number of &quot;Free Fall Events&quot; detected") },
        { -1, QString(), QString() }
    };

    return details;
}

static QString getAttrName(qint32 id)
{
    qint32 idx = 0;

    while (attrDetails()[idx].id != -1) {
        if (attrDetails()[idx].id == id)
            return attrDetails()[idx].name;
        idx++;
    }

    return QString();
}

static QString getAttrDescription(qint32 id)
{
    qint32 idx = 0;

    while (attrDetails()[idx].id != -1) {
        if (attrDetails()[idx].id == id)
            return attrDetails()[idx].desc;
        idx++;
    }

    return QString();
}

static SmartAttribute::Assessment getAssessment(const SmartAttributeParsedData& a)
{
    SmartAttribute::Assessment rval = SmartAttribute::Assessment::NotApplicable;

    bool failed = false;
    bool hasFailed = false;

    if (a.prefailure()) {
        if (a.goodNowValid() && !a.goodNow())
            failed = true;

        if (a.goodInThePastValid() && !a.goodInThePast())
            hasFailed = true;
    } else if (a.thresholdValid()) {
        if (a.currentValueValid() && a.currentValue() <= a.threshold())
            failed = true;
        else if (a.worstValueValid() && a.worstValue() <= a.threshold())
            hasFailed = true;
    }

    if (failed)
        rval = SmartAttribute::Assessment::Failing;
    else if (hasFailed)
        rval = SmartAttribute::Assessment::HasFailed;
    else if (a.warn())
        rval = SmartAttribute::Assessment::Warning;
    else if (a.goodNowValid())
        rval = SmartAttribute::Assessment::Good;

    return rval;
}

static QString getRaw(quint64 raw)
{
    QString rval = QStringLiteral("0x");
    rval += QStringLiteral("%1").arg(raw, 12, 16, QLatin1Char('0'));
    return rval;
}
