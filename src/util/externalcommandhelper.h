/*************************************************************************
 *  Copyright (C) 2017 by Andrius Štikonas <andrius@stikonas.eu>         *
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

#ifndef KPMCORE_EXTERNALCOMMANDHELPER_H

#define KPMCORE_EXTERNALCOMMANDHELPER_H

#include <KAuth>

#include <QString>
#include <QProcess>

using namespace KAuth;

class ExternalCommandHelper : public QObject
{
    Q_OBJECT

Q_SIGNALS:
    void progress(int);

public:
    bool readData(QByteArray& buffer, qint64 offset, qint64 size);
    bool writeData(QByteArray& buffer, qint64 offset);

public Q_SLOTS:
    ActionReply start(const QVariantMap& args);
    ActionReply copyblockshelper(const QVariantMap& args);

private:
    void onReadOutput();

    QString command;
    QString sourceDevice;
    QString targetDevice;
    QProcess cmd;
//     QByteArray output;
};

#endif
