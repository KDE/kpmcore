/*
    SPDX-FileCopyrightText: 2026 Ramil Nurmanov <ramil2004nur@gmail.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "core/smartnvmehealth.h"

#include <QLocale>
#include <QStringList>

#include <KFormat>
#include <KLocalizedString>

#include <limits>

bool SmartNvmeHealth::isEmpty() const
{
    return m_Values.isEmpty();
}

bool SmartNvmeHealth::contains(Field f) const
{
    return m_Values.contains(f);
}

std::optional<quint64> SmartNvmeHealth::value(Field f) const
{
    const auto it = m_Values.constFind(f);
    if (it == m_Values.constEnd())
        return std::nullopt;
    return it.value();
}

void SmartNvmeHealth::setValue(Field f, quint64 v)
{
    m_Values.insert(f, v);
}

std::optional<quint64> SmartNvmeHealth::dataUnitsToBytes(quint64 units)
{
    if (units > std::numeric_limits<quint64>::max() / dataUnitSize)
        return std::nullopt;
    return units * dataUnitSize;
}

QString SmartNvmeHealth::fieldName(Field f)
{
    switch (f) {
    case Field::CriticalWarning:
        return xi18nc("@label NVMe health", "Critical warning");
    case Field::AvailableSpare:
        return xi18nc("@label NVMe health", "Available spare");
    case Field::AvailableSpareThreshold:
        return xi18nc("@label NVMe health", "Available spare threshold");
    case Field::PercentageUsed:
        return xi18nc("@label NVMe health", "Percentage used");
    case Field::DataUnitsRead:
        return xi18nc("@label NVMe health", "Data read");
    case Field::DataUnitsWritten:
        return xi18nc("@label NVMe health", "Data written");
    case Field::HostReadCommands:
        return xi18nc("@label NVMe health", "Host read commands");
    case Field::HostWriteCommands:
        return xi18nc("@label NVMe health", "Host write commands");
    case Field::ControllerBusyTime:
        return xi18nc("@label NVMe health", "Controller busy time");
    case Field::PowerCycles:
        return xi18nc("@label NVMe health", "Power cycles");
    case Field::PowerOnHours:
        return xi18nc("@label NVMe health", "Power on hours");
    case Field::UnsafeShutdowns:
        return xi18nc("@label NVMe health", "Unsafe shutdowns");
    case Field::MediaErrors:
        return xi18nc("@label NVMe health", "Media and data integrity errors");
    case Field::ErrorLogEntries:
        return xi18nc("@label NVMe health", "Error information log entries");
    }

    return QString();
}

QString SmartNvmeHealth::criticalWarningToString(quint64 warning)
{
    const QString hex = QStringLiteral("0x%1").arg(warning, 2, 16, QLatin1Char('0'));

    if (warning == 0)
        return xi18nc("@item:intable NVMe critical warning, %1 is the raw value", "%1 (none)", hex);

    QStringList flags;
    if (warning & SpareBelowThreshold)
        flags << xi18nc("@item:intable NVMe critical warning", "available spare below threshold");
    if (warning & TemperatureThreshold)
        flags << xi18nc("@item:intable NVMe critical warning", "temperature threshold exceeded");
    if (warning & ReliabilityDegraded)
        flags << xi18nc("@item:intable NVMe critical warning", "reliability degraded");
    if (warning & ReadOnly)
        flags << xi18nc("@item:intable NVMe critical warning", "media in read-only mode");
    if (warning & VolatileMemoryBackupFailed)
        flags << xi18nc("@item:intable NVMe critical warning", "volatile memory backup failed");
    if (warning & PersistentMemoryReadOnly)
        flags << xi18nc("@item:intable NVMe critical warning", "persistent memory region read-only");
    if (warning & ~quint64(0x3F))
        flags << xi18nc("@item:intable NVMe critical warning", "unknown warning");

    return xi18nc("@item:intable NVMe critical warning, %1 is the raw value, %2 the list of warnings", "%1: %2", hex, flags.join(QStringLiteral(", ")));
}

QString SmartNvmeHealth::valueToString(Field f) const
{
    const auto v = value(f);
    if (!v)
        return xi18nc("@item:intable", "unknown");

    switch (f) {
    case Field::CriticalWarning:
        return criticalWarningToString(*v);

    case Field::AvailableSpare:
    case Field::AvailableSpareThreshold:
    case Field::PercentageUsed:
        return xi18nc("@item:intable percentage", "%1%", QLocale().toString(*v));

    case Field::DataUnitsRead:
    case Field::DataUnitsWritten: {
        const auto bytes = dataUnitsToBytes(*v);
        if (!bytes)
            return xi18ncp("@item:intable NVMe data units of 512000 bytes", "%1 data unit", "%1 data units", *v);
        return KFormat().formatByteSize(static_cast<double>(*bytes));
    }

    case Field::ControllerBusyTime:
        return xi18ncp("@item:intable", "%1 minute", "%1 minutes", *v);

    case Field::PowerOnHours:
        return xi18ncp("@item:intable", "%1 hour", "%1 hours", *v);

    default:
        return QLocale().toString(*v);
    }
}
