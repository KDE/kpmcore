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

#include <KAuth>

#include <QEventLoop>
#include <QRandomGenerator64>
#include <QString>
#include <QProcess>

#include <QtCrypto>

using namespace KAuth;

class ExternalCommandHelper : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.kpmcore.externalcommand")

Q_SIGNALS:
    void progress(int);
    void quit();

public:
    bool readData(const QString& sourceDevice, QByteArray& buffer, qint64 offset, qint64 size);
    bool writeData(const QString& targetDevice, const QByteArray& buffer, qint64 offset);

public Q_SLOTS:
    ActionReply init(const QVariantMap& args);
    Q_SCRIPTABLE quint64 getNonce();
    Q_SCRIPTABLE QVariantMap start(const QByteArray& signature, const quint64 nonce, const QString& command, const QStringList& arguments, const QByteArray& input, const int processChannelMode);
    Q_SCRIPTABLE QVariantMap copyblocks(const QByteArray& signature, const quint64 nonce, const QString& sourceDevice, const qint64 sourceFirstByte, const qint64 sourceLength, const QString& targetDevice, const qint64 targetFirstByte, const qint64 blockSize);
    Q_SCRIPTABLE bool writeData(const QByteArray& signature, const quint64 nonce, const QByteArray& buffer, const QString& targetDevice, const qint64 targetFirstByte);
    Q_SCRIPTABLE void exit(const QByteArray& signature, const quint64 nonce);

private:
    void onReadOutput();

    std::unique_ptr<QEventLoop> m_loop;
    QCA::Initializer initializer;
    QCA::PublicKey m_publicKey;
    QRandomGenerator64 m_Generator;
    std::unordered_set<quint64> m_Nonces;
    QString m_command;
    QString m_sourceDevice;
    QProcess m_cmd;

//     QByteArray output;
};

#endif
