/*
    SPDX-FileCopyrightText: 2026 Ramil Nurmanov <ramil2004nur@gmail.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "util/devicereadbenchmark.h"

#include "externalcommandhelper_interface.h"

#include <QDBusArgument>
#include <QDBusConnection>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QDebug>
#include <QFileInfo>
#include <QPointer>
#include <QVariantMap>

struct DeviceReadBenchmarkPrivate
{
    QString m_DeviceNode;
    QPointer<QDBusPendingCallWatcher> m_Watcher;
};

DeviceReadBenchmark::DeviceReadBenchmark(const QString& deviceNode, QObject* parent) :
    QObject(parent),
    d(std::make_unique<DeviceReadBenchmarkPrivate>())
{
    const QString canonicalPath = QFileInfo(deviceNode).canonicalFilePath();
    d->m_DeviceNode = canonicalPath.isEmpty() ? deviceNode : canonicalPath;
}

DeviceReadBenchmark::~DeviceReadBenchmark()
{
}

bool DeviceReadBenchmark::isPending() const
{
    return !d->m_Watcher.isNull();
}

bool DeviceReadBenchmark::request(const QList<qint64>& offsets, qint64 length)
{
    if (isPending())
        return false;

    if (!QDBusConnection::systemBus().isConnected()) {
        qWarning() << QDBusConnection::systemBus().lastError().message();
        return false;
    }

    auto *interface = new org::kde::kpmcore::externalcommand(QStringLiteral("org.kde.kpmcore.helperinterface"),
                QStringLiteral("/Helper"), QDBusConnection::systemBus(), this);
    interface->setTimeout(10 * 24 * 3600 * 1000);

    QVariantList offsetList;
    offsetList.reserve(offsets.size());
    for (const qint64 offset : offsets)
        offsetList.append(offset);

    QDBusPendingCall pcall = interface->BenchmarkRead(d->m_DeviceNode, offsetList, length);
    d->m_Watcher = new QDBusPendingCallWatcher(pcall, this);

    connect(d->m_Watcher, &QDBusPendingCallWatcher::finished, this, [this, interface] (QDBusPendingCallWatcher *watcher) {
        watcher->deleteLater();
        interface->deleteLater();

        Result result;
        if (watcher->isError()) {
            qWarning() << watcher->error();
            result.error = watcher->error().message();
        } else {
            const QVariantMap reply = QDBusPendingReply<QVariantMap>(*watcher).value();
            if (!reply.isEmpty()) {
                result.status = static_cast<Status>(reply[QStringLiteral("status")].toInt());
                result.deviceSize = reply[QStringLiteral("deviceSize")].toLongLong();
                result.logicalBlockSize = reply[QStringLiteral("logicalBlockSize")].toLongLong();
                result.physicalBlockSize = reply[QStringLiteral("physicalBlockSize")].toLongLong();
                const QVariantList elapsedNs = qdbus_cast<QVariantList>(reply[QStringLiteral("elapsedNs")]);
                for (const QVariant& value : elapsedNs)
                    result.elapsedNs.append(value.toLongLong());
                result.error = reply[QStringLiteral("error")].toString();
            }
        }

        d->m_Watcher.clear();
        Q_EMIT finished(result);
    });

    return true;
}

#include "moc_devicereadbenchmark.cpp"
