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

#include "core/smartparser.h"

#include "core/smartattributeparseddata.h"
#include "core/smartdiskinformation.h"

#include "util/externalcommand.h"

#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QString>

SmartParser::SmartParser(const QString &device_path) :
    m_DevicePath(device_path),
    m_DiskInformation(nullptr)
{

}

bool SmartParser::init()
{
    loadSmartOutput();

    if (m_SmartOutput.isEmpty())
        return false;

    QJsonObject smartJson = m_SmartOutput.object();

    QString model_name = QString::fromLocal8Bit("model_name");
    QString firmware = QString::fromLocal8Bit("firmware_version");
    QString serial_number = QString::fromLocal8Bit("serial_number");
    QString device = QString::fromLocal8Bit("device");
    QString smart_status = QString::fromLocal8Bit("smart_status");
    QString passed = QString::fromLocal8Bit("passed");
    QString self_test = QString::fromLocal8Bit("self_test");
    QString status = QString::fromLocal8Bit("status");
    QString value = QString::fromLocal8Bit("value");
    QString user_capacity = QString::fromLocal8Bit("user_capacity");

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
    m_DiskInformation->setSize(smartJson[user_capacity].toVariant().toULongLong());

    QJsonObject selfTest = smartJson[self_test].toObject();
    QJsonObject selfTestStatus = selfTest[status].toObject();

    m_DiskInformation->setSelfTestExecutionStatus((SmartDiskInformation::SmartSelfTestExecutionStatus)
                                                  selfTestStatus[value].toInt());

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

void SmartParser::loadSmartOutput()
{
    if (m_SmartOutput.isEmpty()) {
        QStringList args;
        args.append(QString::fromLocal8Bit("--all"));
        args.append(QString::fromLocal8Bit("--json"));
        args.append(devicePath());

        ExternalCommand smartctl(QString::fromLocal8Bit("smartctl"), args);

        if (smartctl.run() && smartctl.exitCode() == 0) {
            QByteArray output = smartctl.rawOutput();

            m_SmartOutput = QJsonDocument::fromJson(output);
        }
        else
            qDebug() << "smartctl initialization failed for " << devicePath() << ": " << strerror(errno);
    }
}

void SmartParser::loadAttributes()
{
    loadSmartOutput();

    if (m_SmartOutput.isEmpty())
        return;

    QJsonObject smartJson = m_SmartOutput.object();

    QString ata_smart_attributes = QString::fromLocal8Bit("ata_smart_attributes");
    QString table = QString::fromLocal8Bit("table");

    QJsonObject ataSmartAttributes = smartJson[ata_smart_attributes].toObject();

    QJsonArray attributeArray = ataSmartAttributes[table].toArray();

    if (!m_DiskInformation) {
        qDebug() << "error loading smart attributes for " << devicePath() << ": " << strerror(errno);
        return;
    }

    for (const QJsonValue &value : qAsConst(attributeArray)) {
        SmartAttributeParsedData parsedObject(m_DiskInformation, value.toObject());
        m_DiskInformation->addAttribute(parsedObject);
    }
}
