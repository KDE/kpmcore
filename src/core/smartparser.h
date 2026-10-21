/*
    SPDX-FileCopyrightText: 2018 Caio Jordão Carvalho <caiojcarvalho@gmail.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KPMCORE_SMARTPARSER_H
#define KPMCORE_SMARTPARSER_H

#include "core/smartstatus.h"

#include <QByteArray>
#include <QJsonDocument>
#include <QString>

class SmartDiskInformation;

/** A parser to SMART JSON output.

    Responsible to execute smartctl and parse its output.

    @author Caio Jordão Carvalho <caiojcarvalho@gmail.com>
*/
class SmartParser
{
public:
    enum ExitStatusBit {
        CommandLineError = 0x01,
        DeviceOpenError = 0x02,
        DeviceCommandError = 0x04,
        DiskFailing = 0x08,
        PrefailAttributeBelowThreshold = 0x10,
        AttributeBelowThresholdInPast = 0x20,
        ErrorLogHasErrors = 0x40,
        SelfTestLogHasErrors = 0x80,
    };

public:
    explicit SmartParser(const QString &device_path);
    ~SmartParser();

public:
    bool init();
    bool init(const QByteArray &output, int exitStatus);

public:
    const QString &devicePath() const
    {
        return m_DevicePath; /**< @return the device path that SMART must analyze */
    }

    SmartDiskInformation *diskInformation() const
    {
        return m_DiskInformation; /**< @return a reference to parsed disk information */
    }

protected:
    bool loadSmartOutput();

    bool parseSmartOutput(const QByteArray &output);

    bool loadDiskInformation();

    void loadAttributes();

    void loadNvmeHealth();

    void loadCommonValues();

    SmartStatus::SelfTestStatus loadSelfTestStatus() const;

private:
    const QString m_DevicePath;
    QJsonDocument m_SmartOutput;
    int m_ExitStatus;
    SmartDiskInformation *m_DiskInformation;

};

#endif // SMARTPARSER_H
