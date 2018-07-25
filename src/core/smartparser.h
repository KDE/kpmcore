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

#ifndef KPMCORE_SMARTPARSER_H
#define KPMCORE_SMARTPARSER_H

#include <QJsonDocument>
#include <QString>

class SmartDiskInformation;

/** A parser to SMART JSON output.

    Responsible to execute smartctl and parse its output.

    @author Caio Carvalho <caiojcarvalho@gmail.com>
*/
class SmartParser
{
public:
    SmartParser(const QString &device_path);
    ~SmartParser();

public:
    bool init();

public:
    const QString &devicePath() const
    {
        return m_DevicePath; /**< @return the device path that SMART must analyse */
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
