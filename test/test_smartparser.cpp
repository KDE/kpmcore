/*
    SPDX-FileCopyrightText: 2026 Ramil Nurmanov <ramil2004nur@gmail.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include <QObject>

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QtTest>

#include "core/smartattributeparseddata.h"
#include "core/smartdiskinformation.h"
#include "core/smartnvmehealth.h"
#include "core/smartparser.h"
#include "core/smartstatus.h"

#include <limits>

using namespace Qt::StringLiterals;

using Field = SmartNvmeHealth::Field;
using SelfTest = SmartStatus::SelfTestStatus;

Q_DECLARE_METATYPE(SmartStatus::SelfTestStatus)

static QByteArray fixture(const QString &name)
{
    QFile file(QFINDTESTDATA(u"test_smartparser_data/"_s + name));
    if (!file.open(QIODevice::ReadOnly))
        qFatal("cannot open %s", qPrintable(name));
    return file.readAll();
}

static QJsonObject fixtureObject(const QString &name)
{
    return QJsonDocument::fromJson(fixture(name)).object();
}

static QByteArray toJson(const QJsonObject &object)
{
    return QJsonDocument(object).toJson();
}

static void setNested(QJsonObject &object, const QString &outer, const QString &inner, const QJsonValue &value)
{
    QJsonObject nested = object[outer].toObject();
    if (value.isUndefined())
        nested.remove(inner);
    else
        nested[inner] = value;
    object[outer] = nested;
}

class SmartParserTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void nvmeQemuZeroCounters()
    {
        SmartParser parser(u"/dev/nvme0n1"_s);
        QVERIFY(parser.init(fixture(u"nvme_qemu.json"_s), 0));

        const SmartDiskInformation *disk = parser.diskInformation();
        QVERIFY(disk);
        QVERIFY(disk->isNvme());
        QCOMPARE(disk->model(), u"QEMU NVMe Ctrl"_s);
        QCOMPARE(disk->firmware(), u"7.2.22"_s);
        QVERIFY(disk->smartStatus());
        QCOMPARE(disk->overall(), SmartStatus::Overall::Good);

        QVERIFY(disk->temperatureValid());
        QCOMPARE(disk->temperature(), 323150ULL);

        QVERIFY(disk->poweredOnValid());
        QCOMPARE(disk->poweredOn(), 0ULL);
        QVERIFY(disk->powerCyclesValid());
        QCOMPARE(disk->powerCycles(), 0ULL);

        QVERIFY(!disk->badSectorsValid());
        QVERIFY(disk->attributes().isEmpty());

        QCOMPARE(disk->selfTestExecutionStatus(), SelfTest::Unknown);

        const SmartNvmeHealth &health = disk->nvmeHealth();
        for (const Field f : { Field::CriticalWarning, Field::AvailableSpare, Field::AvailableSpareThreshold, Field::PercentageUsed,
                               Field::DataUnitsRead, Field::DataUnitsWritten, Field::HostReadCommands, Field::HostWriteCommands,
                               Field::ControllerBusyTime, Field::PowerCycles, Field::PowerOnHours, Field::UnsafeShutdowns,
                               Field::MediaErrors, Field::ErrorLogEntries }) {
            QVERIFY2(health.contains(f), qPrintable(SmartNvmeHealth::fieldName(f)));
            QCOMPARE(health.value(f), std::optional<quint64>(0));
        }
    }

    void nvmeMissingFields()
    {
        SmartParser parser(u"/dev/nvme1n1"_s);
        QVERIFY(parser.init(fixture(u"nvme_missing_fields.json"_s), 0));

        const SmartDiskInformation *disk = parser.diskInformation();
        QVERIFY(disk->isNvme());
        QVERIFY(!disk->temperatureValid());
        QVERIFY(!disk->poweredOnValid());
        QVERIFY(!disk->powerCyclesValid());
        QCOMPARE(disk->overall(), SmartStatus::Overall::Good);
        QCOMPARE(disk->selfTestExecutionStatus(), SelfTest::NoResults);

        const SmartNvmeHealth &health = disk->nvmeHealth();
        QCOMPARE(health.value(Field::AvailableSpare), std::optional<quint64>(100));
        QCOMPARE(health.value(Field::AvailableSpareThreshold), std::optional<quint64>(10));
        QVERIFY(!health.contains(Field::DataUnitsRead));
        QVERIFY(!health.contains(Field::MediaErrors));
        QVERIFY(!health.value(Field::MediaErrors).has_value());
        QVERIFY(health.valueToString(Field::MediaErrors) != health.valueToString(Field::AvailableSpare));
    }

    void nvmeWithoutHealthLog()
    {
        QJsonObject json = fixtureObject(u"nvme_qemu.json"_s);
        json.remove(u"nvme_smart_health_information_log"_s);
        json.remove(u"temperature"_s);

        SmartParser parser(u"/dev/nvme0n1"_s);
        QVERIFY(parser.init(toJson(json), 0));

        const SmartDiskInformation *disk = parser.diskInformation();
        QVERIFY(disk->isNvme());
        QVERIFY(disk->nvmeHealth().isEmpty());
        QVERIFY(!disk->badSectorsValid());
        QVERIFY(!disk->temperatureValid());
        QVERIFY(disk->poweredOnValid());
        QVERIFY(disk->powerCyclesValid());
    }

    void nvmeTemperatureFromHealthLog()
    {
        QJsonObject json = fixtureObject(u"nvme_qemu.json"_s);
        json.remove(u"temperature"_s);
        json.remove(u"power_on_time"_s);
        json.remove(u"power_cycle_count"_s);
        setNested(json, u"nvme_smart_health_information_log"_s, u"power_on_hours"_s, 7);
        setNested(json, u"nvme_smart_health_information_log"_s, u"power_cycles"_s, 3);

        SmartParser parser(u"/dev/nvme0n1"_s);
        QVERIFY(parser.init(toJson(json), 0));

        const SmartDiskInformation *disk = parser.diskInformation();
        QVERIFY(disk->temperatureValid());
        QCOMPARE(disk->temperature(), 323150ULL);
        QCOMPARE(disk->poweredOn(), 7ULL * 60 * 60 * 1000);
        QCOMPARE(disk->powerCycles(), 3ULL);
    }

    void nvmeSelfTestInProgress()
    {
        QJsonObject json = fixtureObject(u"nvme_warnings.json"_s);
        QJsonObject log = json[u"nvme_self_test_log"_s].toObject();
        log[u"current_self_test_operation"_s] = QJsonObject{ { u"value"_s, 1 }, { u"string"_s, u"Short self-test in progress"_s } };
        json[u"nvme_self_test_log"_s] = log;

        SmartParser parser(u"/dev/nvme2n1"_s);
        QVERIFY(parser.init(toJson(json), 0));
        QCOMPARE(parser.diskInformation()->selfTestExecutionStatus(), SelfTest::InProgress);
    }

    void nvmeWarningsAndErrors()
    {
        const int exitStatus = SmartParser::DeviceCommandError | SmartParser::DiskFailing | SmartParser::ErrorLogHasErrors;

        SmartParser parser(u"/dev/nvme2n1"_s);
        QVERIFY(parser.init(fixture(u"nvme_warnings.json"_s), exitStatus));

        const SmartDiskInformation *disk = parser.diskInformation();
        QVERIFY(!disk->smartStatus());
        QCOMPARE(disk->overall(), SmartStatus::Overall::Bad);
        QCOMPARE(disk->selfTestExecutionStatus(), SelfTest::ErrorUnknown);
        QVERIFY(!disk->badSectorsValid());
        QCOMPARE(disk->badSectors(), 0ULL);
        QCOMPARE(disk->temperature(), 314150ULL);
        QCOMPARE(disk->poweredOn(), 26000ULL * 60 * 60 * 1000);
        QCOMPARE(disk->powerCycles(), 1500ULL);

        const SmartNvmeHealth &health = disk->nvmeHealth();
        QCOMPARE(health.value(Field::CriticalWarning), std::optional<quint64>(5));
        QCOMPARE(health.value(Field::AvailableSpare), std::optional<quint64>(4));
        QCOMPARE(health.value(Field::PercentageUsed), std::optional<quint64>(117));
        QCOMPARE(health.value(Field::MediaErrors), std::optional<quint64>(3));
        QCOMPARE(health.value(Field::ErrorLogEntries), std::optional<quint64>(12));
        QCOMPARE(health.value(Field::UnsafeShutdowns), std::optional<quint64>(87));
        QCOMPARE(health.value(Field::ControllerBusyTime), std::optional<quint64>(4321));

        QCOMPARE(health.value(Field::DataUnitsRead), std::optional<quint64>(12345678901234567ULL));
        QCOMPARE(health.value(Field::HostWriteCommands), std::optional<quint64>(9007199254740993ULL));
        QVERIFY(!health.contains(Field::HostReadCommands));

        QCOMPARE(SmartNvmeHealth::dataUnitsToBytes(*health.value(Field::DataUnitsWritten)), std::optional<quint64>(512000000000ULL));
        QVERIFY(!SmartNvmeHealth::dataUnitsToBytes(*health.value(Field::DataUnitsRead)).has_value());
    }

    void nvmeTemperatureWarning()
    {
        QJsonObject json = fixtureObject(u"nvme_qemu.json"_s);
        setNested(json, u"nvme_smart_health_information_log"_s, u"critical_warning"_s, int(SmartNvmeHealth::TemperatureThreshold));
        setNested(json, u"smart_status"_s, u"passed"_s, false);

        SmartParser parser(u"/dev/nvme0n1"_s);
        QVERIFY(parser.init(toJson(json), SmartParser::DiskFailing));
        QCOMPARE(parser.diskInformation()->overall(), SmartStatus::Overall::BadNow);
    }

    void exitStatusNoData_data()
    {
        QTest::addColumn<int>("exitStatus");
        QTest::newRow("command line") << int(SmartParser::CommandLineError);
        QTest::newRow("device open") << int(SmartParser::DeviceOpenError);
        QTest::newRow("device open with disk bits") << int(SmartParser::DeviceOpenError | SmartParser::DiskFailing);
    }

    void exitStatusNoData()
    {
        QFETCH(int, exitStatus);
        SmartParser parser(u"/dev/nvme0n1"_s);
        QVERIFY(!parser.init(fixture(u"nvme_qemu.json"_s), exitStatus));
        QVERIFY(!parser.diskInformation());
    }

    void exitStatusDiskBits()
    {
        for (int bit = 2; bit < 8; ++bit) {
            SmartParser parser(u"/dev/sda"_s);
            QVERIFY2(parser.init(fixture(u"ata.json"_s), 1 << bit), qPrintable(QString::number(bit)));
            QVERIFY(parser.diskInformation());
        }
    }

    void invalidOutput_data()
    {
        QTest::addColumn<QByteArray>("output");
        QTest::newRow("empty") << QByteArray();
        QTest::newRow("truncated") << fixture(u"nvme_qemu.json"_s).left(200);
        QTest::newRow("array") << QByteArray("[1, 2]");
        QTest::newRow("text") << QByteArray("smartctl: command not found\n");
    }

    void invalidOutput()
    {
        QFETCH(QByteArray, output);
        SmartParser parser(u"/dev/nvme0n1"_s);
        QVERIFY(!parser.init(output, 0));
    }

    void missingIdentification_data()
    {
        QTest::addColumn<QString>("key");
        QTest::newRow("device") << u"device"_s;
        QTest::newRow("smart_status") << u"smart_status"_s;
        QTest::newRow("model_name") << u"model_name"_s;
        QTest::newRow("serial_number") << u"serial_number"_s;
        QTest::newRow("firmware_version") << u"firmware_version"_s;
    }

    void missingIdentification()
    {
        QFETCH(QString, key);
        QJsonObject json = fixtureObject(u"nvme_qemu.json"_s);
        json.remove(key);

        SmartParser parser(u"/dev/nvme0n1"_s);
        QVERIFY(!parser.init(toJson(json), 0));
    }

    void ataRegression()
    {
        SmartParser parser(u"/dev/sda"_s);
        QVERIFY(parser.init(fixture(u"ata.json"_s), 0));

        const SmartDiskInformation *disk = parser.diskInformation();
        QVERIFY(!disk->isNvme());
        QVERIFY(disk->nvmeHealth().isEmpty());
        QCOMPARE(disk->model(), u"WDC WD10EZEX-08WN4A0"_s);
        QCOMPARE(disk->sectors(), 1000204886016ULL);
        QCOMPARE(int(disk->attributes().size()), 6);

        QVERIFY(disk->badSectorsValid());
        QCOMPARE(disk->badSectors(), 10ULL);
        QCOMPARE(disk->overall(), SmartStatus::Overall::BadSectors);

        QVERIFY(disk->temperatureValid());
        QCOMPARE(disk->temperature(), 308150ULL);
        QCOMPARE(disk->poweredOn(), 12345ULL * 60 * 60 * 1000);
        QCOMPARE(disk->powerCycles(), 321ULL);
        QCOMPARE(disk->selfTestExecutionStatus(), SelfTest::Success);
    }

    void ataCommonFieldsFallback()
    {
        QJsonObject json = fixtureObject(u"ata.json"_s);
        json.remove(u"ata_smart_attributes"_s);

        SmartParser parser(u"/dev/sda"_s);
        QVERIFY(parser.init(toJson(json), 0));

        const SmartDiskInformation *disk = parser.diskInformation();
        QVERIFY(disk->attributes().isEmpty());
        QCOMPARE(disk->badSectors(), 0ULL);
        QCOMPARE(disk->overall(), SmartStatus::Overall::Good);
        QCOMPARE(disk->temperature(), 308150ULL);
        QCOMPARE(disk->poweredOn(), 12345ULL * 60 * 60 * 1000);
        QCOMPARE(disk->powerCycles(), 321ULL);
    }

    void ataSelfTest_data()
    {
        QTest::addColumn<QJsonValue>("statusValue");
        QTest::addColumn<QJsonValue>("supported");
        QTest::addColumn<QJsonValue>("log");
        QTest::addColumn<SelfTest>("expected");

        const QJsonValue keep(QJsonValue::Undefined);
        const QJsonValue none(QJsonValue::Null);
        const QJsonObject emptyStandard{ { u"standard"_s, QJsonObject{ { u"revision"_s, 1 }, { u"count"_s, 0 } } } };
        const QJsonObject emptyTables{ { u"standard"_s, QJsonObject{ { u"revision"_s, 1 }, { u"table"_s, QJsonArray() } } },
                                       { u"extended"_s, QJsonObject{ { u"revision"_s, 1 }, { u"table"_s, QJsonArray() } } } };
        const QJsonObject extendedOnly{ { u"extended"_s, QJsonObject{ { u"revision"_s, 1 }, { u"count"_s, 1 },
                                                                   { u"table"_s, QJsonArray{ QJsonObject{ { u"lifetime_hours"_s, 100 } } } } } } };
        const QJsonObject noCount{ { u"standard"_s, QJsonObject{ { u"revision"_s, 1 } } } };

        QTest::newRow("success") << QJsonValue(0) << QJsonValue(true) << keep << SelfTest::Success;
        QTest::newRow("never run") << QJsonValue(0) << QJsonValue(true) << QJsonValue(emptyStandard) << SelfTest::NoResults;
        QTest::newRow("empty tables") << QJsonValue(0) << QJsonValue(true) << QJsonValue(emptyTables) << SelfTest::NoResults;
        QTest::newRow("extended log") << QJsonValue(0) << QJsonValue(true) << QJsonValue(extendedOnly) << SelfTest::Success;
        QTest::newRow("no log") << QJsonValue(0) << QJsonValue(true) << none << SelfTest::Unknown;
        QTest::newRow("log without entries info") << QJsonValue(0) << QJsonValue(true) << QJsonValue(noCount) << SelfTest::Unknown;
        QTest::newRow("aborted") << QJsonValue(0x10) << QJsonValue(true) << keep << SelfTest::Aborted;
        QTest::newRow("read error") << QJsonValue(0x73) << QJsonValue(true) << keep << SelfTest::ErrorRead;
        QTest::newRow("in progress") << QJsonValue(0xF4) << QJsonValue(true) << keep << SelfTest::InProgress;
        QTest::newRow("reserved") << QJsonValue(0x90) << QJsonValue(true) << keep << SelfTest::Unknown;
        QTest::newRow("not supported") << QJsonValue(0) << QJsonValue(false) << keep << SelfTest::NotSupported;
        QTest::newRow("no status") << QJsonValue(QJsonValue::Undefined) << QJsonValue(true) << keep << SelfTest::Unknown;
    }

    void ataSelfTest()
    {
        QFETCH(QJsonValue, statusValue);
        QFETCH(QJsonValue, supported);
        QFETCH(QJsonValue, log);
        QFETCH(SelfTest, expected);

        QJsonObject json = fixtureObject(u"ata.json"_s);
        QJsonObject data = json[u"ata_smart_data"_s].toObject();
        setNested(data, u"capabilities"_s, u"self_tests_supported"_s, supported);
        QJsonObject selfTest = data[u"self_test"_s].toObject();
        setNested(selfTest, u"status"_s, u"value"_s, statusValue);
        data[u"self_test"_s] = selfTest;
        json[u"ata_smart_data"_s] = data;

        if (log.isNull())
            json.remove(u"ata_smart_self_test_log"_s);
        else if (log.isObject())
            json[u"ata_smart_self_test_log"_s] = log;

        SmartParser parser(u"/dev/sda"_s);
        QVERIFY(parser.init(toJson(json), 0));
        QCOMPARE(parser.diskInformation()->selfTestExecutionStatus(), expected);
    }

    void ataWithoutSmartData()
    {
        QJsonObject json = fixtureObject(u"ata.json"_s);
        json.remove(u"ata_smart_data"_s);

        SmartParser parser(u"/dev/sda"_s);
        QVERIFY(parser.init(toJson(json), 0));
        QCOMPARE(parser.diskInformation()->selfTestExecutionStatus(), SelfTest::Unknown);
    }

    void dataUnitsToBytes()
    {
        constexpr quint64 max = std::numeric_limits<quint64>::max();
        QCOMPARE(SmartNvmeHealth::dataUnitsToBytes(0), std::optional<quint64>(0));
        QCOMPARE(SmartNvmeHealth::dataUnitsToBytes(1), std::optional<quint64>(512000));
        QCOMPARE(SmartNvmeHealth::dataUnitsToBytes(max / 512000), std::optional<quint64>(max / 512000 * 512000));
        QVERIFY(!SmartNvmeHealth::dataUnitsToBytes(max / 512000 + 1).has_value());
    }

    void criticalWarningString()
    {
        QVERIFY(SmartNvmeHealth::criticalWarningToString(0).startsWith(u"0x00"_s));
        QVERIFY(SmartNvmeHealth::criticalWarningToString(5).startsWith(u"0x05"_s));
        QVERIFY(SmartStatus::tempToString(0) != SmartStatus::tempToString(273150));
    }
};

QTEST_GUILESS_MAIN(SmartParserTest)

#include "test_smartparser.moc"
