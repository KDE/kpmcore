/*
    SPDX-FileCopyrightText: 2026 Ramil Nurmanov <ramil2004nur@gmail.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KPMCORE_DEVICEREADBENCHMARK_H
#define KPMCORE_DEVICEREADBENCHMARK_H

#include "util/libpartitionmanagerexport.h"

#include <QList>
#include <QObject>
#include <QString>
#include <QtGlobal>

#include <memory>

struct DeviceReadBenchmarkPrivate;

class LIBKPMCORE_EXPORT DeviceReadBenchmark : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(DeviceReadBenchmark)

public:
    enum class Status : int {
        Success = 0,
        Unsupported,
        InvalidRequest,
        Failed
    };

    struct Result {
        Status status = Status::Failed;
        qint64 deviceSize = 0;
        qint64 logicalBlockSize = 0;
        qint64 physicalBlockSize = 0;
        QList<qint64> elapsedNs;
        QString error;
    };

    static constexpr qint64 maxReadLength = 16 * 1024 * 1024;
    static constexpr int maxReadsPerRequest = 256;
    static constexpr qint64 maxBytesPerRequest = 256 * 1024 * 1024;

    explicit DeviceReadBenchmark(const QString& deviceNode, QObject* parent = nullptr);
    ~DeviceReadBenchmark() override;

    bool request(const QList<qint64>& offsets, qint64 length);

    bool isPending() const;

Q_SIGNALS:
    void finished(const DeviceReadBenchmark::Result& result);

private:
    std::unique_ptr<DeviceReadBenchmarkPrivate> d;
};

#endif
