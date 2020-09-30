/*
    SPDX-FileCopyrightText: 2017-2020 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2018 Huzaifa Faruqui <huzaifafaruqui@gmail.com>
    SPDX-FileCopyrightText: 2018 Caio Jordão Carvalho <caiojcarvalho@gmail.com>
    SPDX-FileCopyrightText: 2019 Shubham Jangra <aryan100jangid@gmail.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KPMCORE_EXTERNALCOMMANDHELPER_H
#define KPMCORE_EXTERNALCOMMANDHELPER_H

#include <memory>
#include <unordered_set>

#include <KAuth>

#include <QEventLoop>
#include <QString>
#include <QProcess>

using namespace KAuth;

class ExternalCommandHelper : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.kpmcore.externalcommand")

Q_SIGNALS:
    void progress(int);
    void quit();

public:
    bool readData(const QString& sourceDevice, QByteArray& buffer, const qint64 offset, const qint64 size);
    bool writeData(const QString& targetDevice, const QByteArray& buffer, const qint64 offset);
    bool createFile(const QString& filePath, const QByteArray& fileContents);

public Q_SLOTS:
    ActionReply init(const QVariantMap& args);
    Q_SCRIPTABLE QVariantMap start(const QString& command, const QStringList& arguments, const QByteArray& input, const int processChannelMode);
    Q_SCRIPTABLE QVariantMap copyblocks(const QString& sourceDevice, const qint64 sourceFirstByte, const qint64 sourceLength, const QString& targetDevice, const qint64 targetFirstByte, const qint64 blockSize);
    Q_SCRIPTABLE bool writeData(const QByteArray& buffer, const QString& targetDevice, const qint64 targetFirstByte);
    Q_SCRIPTABLE bool createFile(const QByteArray& fileContents, const QString& filePath);
    Q_SCRIPTABLE void exit();

private:
    void onReadOutput();

    std::unique_ptr<QEventLoop> m_loop;
    QProcess m_cmd;
//  QByteArray output;
};

#endif
