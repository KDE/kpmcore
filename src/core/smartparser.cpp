/*
    SPDX-FileCopyrightText: 2018 Caio Jordão Carvalho <caiojcarvalho@gmail.com>
    SPDX-FileCopyrightText: 2020 Andrius Štikonas <andrius@stikonas.eu>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "core/smartparser.h"

#include "core/smartattributeparseddata.h"
#include "core/smartdiskinformation.h"
#include "core/smartnvmehealth.h"

#include "util/externalcommand.h"

#include <limits>
#include <optional>
#include <utility>

#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QString>
#include <QVariant>

static std::optional<quint64> readUnsigned(const QJsonObject &object, const QString &key)
{
    const QJsonValue str = object.value(key + QStringLiteral("_s"));
    if (str.isString()) {
        bool ok = false;
        const quint64 v = str.toString().toULongLong(&ok);
        if (ok)
            return v;
        qDebug() << "smartctl value" << key << "is out of range:" << str.toString();
        return std::nullopt;
    }

    const QJsonValue value = object.value(key);
    if (!value.isDouble())
        return std::nullopt;

    const QVariant v = value.toVariant();
    if (v.userType() == QMetaType::LongLong) {
        const qlonglong n = v.toLongLong();
        if (n < 0)
            return std::nullopt;
        return static_cast<quint64>(n);
    }

    const double d = value.toDouble();
    if (d < 0 || d > 9007199254740992.0 || d != static_cast<double>(static_cast<quint64>(d)))
        return std::nullopt;
    return static_cast<quint64>(d);
}

static std::optional<qint64> readInteger(const QJsonObject &object, const QString &key)
{
    const QJsonValue value = object.value(key);
    if (!value.isDouble())
        return std::nullopt;

    const double d = value.toDouble();
    if (d < -9007199254740992.0 || d > 9007199254740992.0 || d != static_cast<double>(static_cast<qint64>(d)))
        return std::nullopt;
    return static_cast<qint64>(d);
}

static std::optional<quint64> celsiusToMilliKelvin(std::optional<qint64> celsius)
{
    if (!celsius || *celsius <= -273 || *celsius >= 1000)
        return std::nullopt;
    return static_cast<quint64>(*celsius * 1000 + 273150);
}

/** Creates a new SmartParser object
    @param device_path device path that indicates the device that SMART must analyze
*/
SmartParser::SmartParser(const QString &device_path) :
    m_DevicePath(device_path),
    m_ExitStatus(0),
    m_DiskInformation(nullptr)
{
}

SmartParser::~SmartParser()
{
    delete m_DiskInformation;
}

/** Initialize SmartParser data, retrieve the information from SMART JSON and initialize the disk information data */
bool SmartParser::init()
{
    if (!loadSmartOutput())
        return false;

    return loadDiskInformation();
}

bool SmartParser::init(const QByteArray &output, int exitStatus)
{
    m_ExitStatus = exitStatus;

    if (!parseSmartOutput(output))
        return false;

    return loadDiskInformation();
}

/** Run smartctl command and recover its output */
bool SmartParser::loadSmartOutput()
{
    ExternalCommand smartctl(QStringLiteral("smartctl"), { QStringLiteral("--all"), QStringLiteral("--json"), devicePath() }, QProcess::SeparateChannels);

    if (!smartctl.run()) {
        qDebug() << "could not run smartctl for" << devicePath();
        return false;
    }

    m_ExitStatus = smartctl.exitCode();

    return parseSmartOutput(smartctl.rawOutput());
}

bool SmartParser::parseSmartOutput(const QByteArray &output)
{
    QJsonParseError error;
    const QJsonDocument document = QJsonDocument::fromJson(output, &error);

    if (document.isObject()) {
        const QJsonArray messages = document.object()[QStringLiteral("smartctl")].toObject()[QStringLiteral("messages")].toArray();
        for (const QJsonValue &message : messages)
            qDebug() << "smartctl" << devicePath() << message.toObject()[QStringLiteral("severity")].toString()
                     << message.toObject()[QStringLiteral("string")].toString();
    }

    // Exit status of smartctl is a bitfield, check that bits 0 and 1 are not set:
    //  - bit 0: command line did not parse;
    //  - bit 1: device open failed.
    // See `man 8 smartctl` for more details.
    if (m_ExitStatus & (CommandLineError | DeviceOpenError)) {
        qDebug() << "smartctl could not read" << devicePath() << "exit status" << m_ExitStatus;
        return false;
    }

    if (m_ExitStatus)
        qDebug() << "smartctl reported problems for" << devicePath() << "exit status" << m_ExitStatus;

    if (error.error != QJsonParseError::NoError) {
        qDebug() << "could not parse smartctl output for" << devicePath() << ":" << error.errorString();
        return false;
    }

    if (!document.isObject()) {
        qDebug() << "smartctl output for" << devicePath() << "is not a JSON object";
        return false;
    }

    m_SmartOutput = document;
    return true;
}

bool SmartParser::loadDiskInformation()
{
    const QJsonObject smartJson = m_SmartOutput.object();

    for (const auto &key : { QStringLiteral("device"), QStringLiteral("smart_status"), QStringLiteral("model_name"),
                             QStringLiteral("firmware_version"), QStringLiteral("serial_number") }) {
        if (!smartJson.contains(key)) {
            qDebug() << "smartctl output for" << devicePath() << "has no" << key;
            return false;
        }
    }

    delete m_DiskInformation;
    m_DiskInformation = new SmartDiskInformation();

    m_DiskInformation->setSmartStatus(smartJson[QStringLiteral("smart_status")].toObject()[QStringLiteral("passed")].toBool());

    m_DiskInformation->setModel(smartJson[QStringLiteral("model_name")].toString());
    m_DiskInformation->setFirmware(smartJson[QStringLiteral("firmware_version")].toString());
    m_DiskInformation->setSerial(smartJson[QStringLiteral("serial_number")].toString());

    m_DiskInformation->setSectors(readUnsigned(smartJson[QStringLiteral("user_capacity")].toObject(), QStringLiteral("bytes")).value_or(0));

    loadNvmeHealth();

    m_DiskInformation->setSelfTestExecutionStatus(loadSelfTestStatus());

    loadAttributes();

    m_DiskInformation->updateBadSectors();

    m_DiskInformation->updateOverall();

    m_DiskInformation->updateTemperature();
    m_DiskInformation->updatePowerOn();
    m_DiskInformation->updatePowerCycle();

    loadCommonValues();

    if (!m_DiskInformation->temperatureValid())
        qDebug() << "smartctl output for" << devicePath() << "has no temperature";

    if (!m_DiskInformation->poweredOnValid())
        qDebug() << "smartctl output for" << devicePath() << "has no power on time";

    if (!m_DiskInformation->powerCyclesValid())
        qDebug() << "smartctl output for" << devicePath() << "has no power cycle count";

    return true;
}

/** Load SMART disk attributes from JSON data */
void SmartParser::loadAttributes()
{
    if (!m_DiskInformation)
        return;

    QJsonObject smartJson = m_SmartOutput.object();

    QString ata_smart_attributes = QStringLiteral("ata_smart_attributes");
    QString table = QStringLiteral("table");

    QJsonObject ataSmartAttributes = smartJson[ata_smart_attributes].toObject();

    QJsonArray attributeArray = ataSmartAttributes[table].toArray();

    for (const QJsonValue &value : std::as_const(attributeArray)) {
        SmartAttributeParsedData parsedObject(m_DiskInformation, value.toObject());
        m_DiskInformation->addAttribute(parsedObject);
    }
}

void SmartParser::loadNvmeHealth()
{
    const QJsonObject smartJson = m_SmartOutput.object();
    const QString nvme_log = QStringLiteral("nvme_smart_health_information_log");

    const bool nvme = smartJson[QStringLiteral("device")].toObject()[QStringLiteral("protocol")].toString() == QStringLiteral("NVMe");
    if (!nvme && !smartJson.contains(nvme_log))
        return;

    const QJsonObject log = smartJson[nvme_log].toObject();
    if (log.isEmpty())
        qDebug() << "smartctl output for NVMe device" << devicePath() << "has no" << nvme_log;

    using Field = SmartNvmeHealth::Field;
    static const std::pair<Field, const char *> fields[] = {
        { Field::CriticalWarning, "critical_warning" },
        { Field::AvailableSpare, "available_spare" },
        { Field::AvailableSpareThreshold, "available_spare_threshold" },
        { Field::PercentageUsed, "percentage_used" },
        { Field::DataUnitsRead, "data_units_read" },
        { Field::DataUnitsWritten, "data_units_written" },
        { Field::HostReadCommands, "host_reads" },
        { Field::HostWriteCommands, "host_writes" },
        { Field::ControllerBusyTime, "controller_busy_time" },
        { Field::PowerCycles, "power_cycles" },
        { Field::PowerOnHours, "power_on_hours" },
        { Field::UnsafeShutdowns, "unsafe_shutdowns" },
        { Field::MediaErrors, "media_errors" },
        { Field::ErrorLogEntries, "num_err_log_entries" },
    };

    SmartNvmeHealth health;
    for (const auto &[field, key] : fields) {
        if (const auto v = readUnsigned(log, QString::fromLatin1(key)))
            health.setValue(field, *v);
    }

    m_DiskInformation->setNvmeHealth(health);
}

void SmartParser::loadCommonValues()
{
    const QJsonObject smartJson = m_SmartOutput.object();
    const SmartNvmeHealth &health = m_DiskInformation->nvmeHealth();

    if (!m_DiskInformation->temperatureValid()) {
        auto mkelvin = celsiusToMilliKelvin(readInteger(smartJson[QStringLiteral("temperature")].toObject(), QStringLiteral("current")));
        if (!mkelvin)
            mkelvin = celsiusToMilliKelvin(readInteger(smartJson[QStringLiteral("nvme_smart_health_information_log")].toObject(), QStringLiteral("temperature")));
        if (mkelvin)
            m_DiskInformation->setTemperature(*mkelvin);
    }

    if (!m_DiskInformation->poweredOnValid()) {
        const QJsonObject powerOnTime = smartJson[QStringLiteral("power_on_time")].toObject();
        auto hours = readUnsigned(powerOnTime, QStringLiteral("hours"));
        quint64 minutes = readUnsigned(powerOnTime, QStringLiteral("minutes")).value_or(0);
        if (!hours) {
            hours = health.value(SmartNvmeHealth::Field::PowerOnHours);
            minutes = 0;
        }

        constexpr quint64 msecPerMinute = 60ULL * 1000ULL;
        constexpr quint64 msecPerHour = 60ULL * msecPerMinute;
        if (hours && *hours < std::numeric_limits<quint64>::max() / msecPerHour && minutes < 60)
            m_DiskInformation->setPoweredOn(*hours * msecPerHour + minutes * msecPerMinute);
        else if (hours)
            qDebug() << "power on time of" << devicePath() << "is out of range";
    }

    if (!m_DiskInformation->powerCyclesValid()) {
        auto cycles = readUnsigned(smartJson, QStringLiteral("power_cycle_count"));
        if (!cycles)
            cycles = health.value(SmartNvmeHealth::Field::PowerCycles);
        if (cycles)
            m_DiskInformation->setPowerCycles(*cycles);
    }
}

SmartStatus::SelfTestStatus SmartParser::loadSelfTestStatus() const
{
    const QJsonObject smartJson = m_SmartOutput.object();

    if (m_DiskInformation->isNvme()) {
        const QString nvme_self_test_log = QStringLiteral("nvme_self_test_log");

        if (!smartJson.contains(nvme_self_test_log))
            return SmartStatus::SelfTestStatus::Unknown;

        const QJsonObject log = smartJson[nvme_self_test_log].toObject();

        if (readUnsigned(log[QStringLiteral("current_self_test_operation")].toObject(), QStringLiteral("value")).value_or(0) != 0)
            return SmartStatus::SelfTestStatus::InProgress;

        const QJsonArray table = log[QStringLiteral("table")].toArray();
        for (const QJsonValue &entry : table) {
            const auto result = readUnsigned(entry.toObject()[QStringLiteral("self_test_result")].toObject(), QStringLiteral("value"));
            if (!result)
                return SmartStatus::SelfTestStatus::Unknown;

            switch (*result) {
            case 0x0:
                return SmartStatus::SelfTestStatus::Success;
            case 0x1:
            case 0x3:
            case 0x4:
            case 0x8:
            case 0x9:
                return SmartStatus::SelfTestStatus::Aborted;
            case 0x2:
                return SmartStatus::SelfTestStatus::Interrupted;
            case 0x5:
                return SmartStatus::SelfTestStatus::Fatal;
            case 0x6:
            case 0x7:
                return SmartStatus::SelfTestStatus::ErrorUnknown;
            case 0xF:
                continue;
            default:
                return SmartStatus::SelfTestStatus::Unknown;
            }
        }

        return SmartStatus::SelfTestStatus::NoResults;
    }

    const QJsonObject ataSmartData = smartJson[QStringLiteral("ata_smart_data")].toObject();

    const QJsonValue supported = ataSmartData[QStringLiteral("capabilities")].toObject()[QStringLiteral("self_tests_supported")];
    if (supported.isBool() && !supported.toBool())
        return SmartStatus::SelfTestStatus::NotSupported;

    const auto value = readUnsigned(ataSmartData[QStringLiteral("self_test")].toObject()[QStringLiteral("status")].toObject(), QStringLiteral("value"));
    if (!value)
        return SmartStatus::SelfTestStatus::Unknown;

    const quint64 status = (*value >> 4) & 0xF;

    if (status == 0) {
        const QJsonObject selfTestLog = smartJson[QStringLiteral("ata_smart_self_test_log")].toObject();
        bool empty = false;

        for (const auto &logType : { QStringLiteral("standard"), QStringLiteral("extended") }) {
            if (!selfTestLog[logType].isObject())
                continue;

            const QJsonObject log = selfTestLog[logType].toObject();
            const QJsonValue table = log[QStringLiteral("table")];
            const auto count = readUnsigned(log, QStringLiteral("count"));

            if ((table.isArray() && !table.toArray().isEmpty()) || count.value_or(0) > 0)
                return SmartStatus::SelfTestStatus::Success;

            if (table.isArray() || count)
                empty = true;
        }

        return empty ? SmartStatus::SelfTestStatus::NoResults : SmartStatus::SelfTestStatus::Unknown;
    }

    if (status <= static_cast<quint64>(SmartStatus::SelfTestStatus::ErrorHandling) || status == static_cast<quint64>(SmartStatus::SelfTestStatus::InProgress))
        return static_cast<SmartStatus::SelfTestStatus>(status);

    return SmartStatus::SelfTestStatus::Unknown;
}
