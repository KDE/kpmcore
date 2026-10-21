/*
    SPDX-FileCopyrightText: 2026 Ramil Nurmanov <ramil2004nur@gmail.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KPMCORE_SMARTNVMEHEALTH_H
#define KPMCORE_SMARTNVMEHEALTH_H

#include "util/libpartitionmanagerexport.h"

#include <QMap>
#include <QString>

#include <optional>

class LIBKPMCORE_EXPORT SmartNvmeHealth
{
public:
    enum class Field {
        CriticalWarning,
        AvailableSpare,
        AvailableSpareThreshold,
        PercentageUsed,
        DataUnitsRead,
        DataUnitsWritten,
        HostReadCommands,
        HostWriteCommands,
        ControllerBusyTime,
        PowerCycles,
        PowerOnHours,
        UnsafeShutdowns,
        MediaErrors,
        ErrorLogEntries,
    };

    enum CriticalWarningFlag {
        SpareBelowThreshold = 0x01,
        TemperatureThreshold = 0x02,
        ReliabilityDegraded = 0x04,
        ReadOnly = 0x08,
        VolatileMemoryBackupFailed = 0x10,
        PersistentMemoryReadOnly = 0x20,
    };

    static constexpr quint64 dataUnitSize = 512000;

public:
    bool isEmpty() const;
    bool contains(Field f) const;
    std::optional<quint64> value(Field f) const;
    void setValue(Field f, quint64 v);

    QString valueToString(Field f) const;

    static QString fieldName(Field f);
    static QString criticalWarningToString(quint64 warning);
    static std::optional<quint64> dataUnitsToBytes(quint64 units);

private:
    QMap<Field, quint64> m_Values;
};

#endif
