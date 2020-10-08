/*************************************************************************
 *  Copyright (C) 2017-2018 by Andrius Štikonas <andrius@stikonas.eu>    *
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

#include <memory>
#include <unordered_set>

#include <QEventLoop>
#include <QString>
#include <QProcess>
#include <QDBusContext>

class QDBusServiceWatcher;

class ExternalCommandHelper : public QObject, public QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.kpmcore.externalcommand")

Q_SIGNALS:
    Q_SCRIPTABLE void progress(int);
    Q_SCRIPTABLE void report(QString);

public:
    ExternalCommandHelper();
    bool readData(const QString& sourceDevice, QByteArray& buffer, const qint64 offset, const qint64 size);
    bool writeData(const QString& targetDevice, const QByteArray& buffer, const qint64 offset);

public Q_SLOTS:
    Q_SCRIPTABLE QVariantMap start(const QString& command, const QStringList& arguments, const QByteArray& input, const int processChannelMode);
    Q_SCRIPTABLE QVariantMap copyblocks(const QString& sourceDevice, const qint64 sourceFirstByte, const qint64 sourceLength, const QString& targetDevice, const qint64 targetFirstByte, const qint64 blockSize);
    Q_SCRIPTABLE bool writeData(const QByteArray& buffer, const QString& targetDevice, const qint64 targetFirstByte);
    Q_SCRIPTABLE void exit();

private:

    bool isCallerAuthorized();

    void onReadOutput();
    QProcess m_cmd;
    QDBusServiceWatcher *m_serviceWatcher = nullptr;
};

#endif
