/*
    SPDX-FileCopyrightText: 2017-2020 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2018 Huzaifa Faruqui <huzaifafaruqui@gmail.com>
    SPDX-FileCopyrightText: 2018 Caio Jordão Carvalho <caiojcarvalho@gmail.com>
    SPDX-FileCopyrightText: 2019 Shubham Jangra <aryan100jangid@gmail.com>
    SPDX-FileCopyrightText: 2020 David Edmundson <kde@davidedmundson.co.uk>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KPMCORE_EXTERNALCOMMANDHELPER_H
#define KPMCORE_EXTERNALCOMMANDHELPER_H

#include <memory>
#include <unordered_set>

#include <QEventLoop>
#include <QString>
#include <QProcess>
#include <QDBusContext>

class QDBusServiceWatcher;
constexpr qint64 MiB = 1 << 30;

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
    Q_SCRIPTABLE QVariantMap RunCommand(const QString& command, const QStringList& arguments, const QByteArray& input, const int processChannelMode);
    Q_SCRIPTABLE QVariantMap CopyBlocks(const QString& sourceDevice, const qint64 sourceOffset, const qint64 sourceLength,
                                        const QString& targetDevice, const qint64 targetOffset, const qint64 blockSize);
    Q_SCRIPTABLE QByteArray ReadData(const QString& device, const qint64 offset, const qint64 length);
    Q_SCRIPTABLE bool WriteData(const QByteArray& buffer, const QString& targetDevice, const qint64 targetOffset);
    Q_SCRIPTABLE bool CreateFile(const QString& filePath, const QByteArray& fileContents);

private:
    bool isCallerAuthorized();

    void onReadOutput();
    QDBusServiceWatcher *m_serviceWatcher = nullptr;
};

#endif
