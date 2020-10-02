/*
    SPDX-FileCopyrightText: 2018 Caio Jordão Carvalho <caiojcarvalho@gmail.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KPMCORE_SMARTPARSER_H
#define KPMCORE_SMARTPARSER_H

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
    explicit SmartParser(const QString &device_path);
    ~SmartParser();

public:
    bool init();

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
    void loadSmartOutput();

    void loadAttributes();

private:
    const QString m_DevicePath;
    QJsonDocument m_SmartOutput;
    SmartDiskInformation *m_DiskInformation;

};

#endif // SMARTPARSER_H
