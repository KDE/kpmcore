/*
    SPDX-FileCopyrightText: 2018 Caio Jordão Carvalho <caiojcarvalho@gmail.com>
    SPDX-FileCopyrightText: 2020 Andrius Štikonas <andrius@stikonas.eu>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "core/smartparser.h"

#include "core/smartattributeparseddata.h"
#include "core/smartdiskinformation.h"

#include "util/externalcommand.h"

#include <errno.h>
#include <utility>

#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QString>

/** Creates a new SmartParser object
    @param device_path device path that indicates the device that SMART must analyze
*/
SmartParser::SmartParser(const QString &device_path) :
    m_DevicePath(device_path),
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
    loadSmartOutput();

    if (m_SmartOutput.isEmpty())
        return false;

    QJsonObject smartJson = m_SmartOutput.object();

    QString model_name = QStringLiteral("model_name");
    QString firmware = QStringLiteral("firmware_version");
    QString serial_number = QStringLiteral("serial_number");
    QString device = QStringLiteral("device");
    QString smart_status = QStringLiteral("smart_status");
    QString passed = QStringLiteral("passed");
    QString self_test = QStringLiteral("self_test");
    QString status = QStringLiteral("status");
    QString value = QStringLiteral("value");
    QString user_capacity = QStringLiteral("user_capacity");
    QString blocks = QStringLiteral("blocks");

    if (!smartJson.contains(device)) {
        qDebug() << "smart disk open failed for " << devicePath() << ": " << strerror(errno);
        return false;
    }

    if (!smartJson.contains(smart_status)) {
        qDebug() << "getting smart status failed for " << devicePath() << ": " << strerror(errno);
        return false;
    }

    if (!smartJson.contains(model_name) || !smartJson.contains(firmware)
            || !smartJson.contains(serial_number)) {
        qDebug() << "getting disk identification data failed for " << devicePath() << ": " << strerror(
                     errno);
        return false;
    }

    m_DiskInformation = new SmartDiskInformation();

    QJsonObject smartStatus = smartJson[smart_status].toObject();

    m_DiskInformation->setSmartStatus(smartStatus[passed].toBool());

    m_DiskInformation->setModel(smartJson[model_name].toString());
    m_DiskInformation->setFirmware(smartJson[firmware].toString());
    m_DiskInformation->setSerial(smartJson[serial_number].toString());

    const auto user_capacity_object = smartJson[user_capacity].toObject();
    QString user_capacity_blocks = QStringLiteral("bytes");
    m_DiskInformation->setSectors(user_capacity_object[user_capacity_blocks].toVariant().toULongLong());

    QJsonObject selfTest = smartJson[self_test].toObject();
    QJsonObject selfTestStatus = selfTest[status].toObject();

    m_DiskInformation->setSelfTestExecutionStatus(static_cast<SmartStatus::SelfTestStatus>(selfTestStatus[value].toInt()));

    loadAttributes();

    m_DiskInformation->updateBadSectors();

    m_DiskInformation->updateOverall();

    if (!m_DiskInformation->updateTemperature())
        qDebug() << "getting temp failed for " <<  devicePath() << ": " << strerror(errno);

    if (!m_DiskInformation->updatePowerOn())
        qDebug() << "getting powered on time failed for " <<  devicePath() << ": " << strerror(errno);

    if (!m_DiskInformation->updatePowerCycle())
        qDebug() << "getting power cycles failed for " <<  devicePath() << ": " << strerror(errno);

    return true;
}

/** Run smartctl command and recover its output */
void SmartParser::loadSmartOutput()
{
    if (m_SmartOutput.isEmpty()) {
        ExternalCommand smartctl(QStringLiteral("smartctl"), { QStringLiteral("--all"), QStringLiteral("--json"), devicePath() });

        // Exit status of smartctl is a bitfield, check that bits 0 and 1 are not set:
        //  - bit 0: command line did not parse;
        //  - bit 1: device open failed.
        // See `man 8 smartctl` for more details.
        if (smartctl.run() && (smartctl.exitCode() & 1) == 0 && (smartctl.exitCode() & 2) == 0) {
            QByteArray output = smartctl.rawOutput();

            m_SmartOutput = QJsonDocument::fromJson(output);
        }
        else
            qDebug() << "smartctl initialization failed for " << devicePath() << ": " << strerror(errno);
    }
}

/** Load SMART disk attributes from JSON data */
void SmartParser::loadAttributes()
{
    loadSmartOutput();

    if (m_SmartOutput.isEmpty())
        return;

    QJsonObject smartJson = m_SmartOutput.object();

    QString ata_smart_attributes = QStringLiteral("ata_smart_attributes");
    QString table = QStringLiteral("table");

    QJsonObject ataSmartAttributes = smartJson[ata_smart_attributes].toObject();

    QJsonArray attributeArray = ataSmartAttributes[table].toArray();

    if (!m_DiskInformation) {
        qDebug() << "error loading smart attributes for " << devicePath() << ": " << strerror(errno);
        return;
    }

    for (const QJsonValue &value : std::as_const(attributeArray)) {
        SmartAttributeParsedData parsedObject(m_DiskInformation, value.toObject());
        m_DiskInformation->addAttribute(parsedObject);
    }
}
