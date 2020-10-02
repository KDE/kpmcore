/*
    SPDX-FileCopyrightText: 2008-2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2012-2019 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2015 Chris Campbell <c.j.campbell@ed.ac.uk>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KPMCORE_CAPACITY_H
#define KPMCORE_CAPACITY_H

#include "util/libpartitionmanagerexport.h"

class Partition;
class Device;

#include <QtGlobal>

/** Represent any kind of capacity.

    Any kind of capacity that can be expressed in units of Byte, KiB, MiB and so on. Also prints
    capacities in nicely formatted ways.

    @author Volker Lanz <vl@fidra.de>
*/
class LIBKPMCORE_EXPORT Capacity
{
public:
    /** Units we can deal with */
    enum class Unit : uint8_t { Byte, KiB, MiB, GiB, TiB, PiB, EiB, ZiB, YiB };
    /** Type of capacity to print */
    enum class Type : uint8_t { Used, Available, Total };
    /** Flags for printing */
    enum class Flag : uint8_t { NoFlags = 0, AppendUnit = 1, AppendBytes = 2 };
    Q_DECLARE_FLAGS(Flags, Flag)

public:
    explicit Capacity(qint64 size);
    explicit Capacity(const Partition& p, Type t = Type::Total);
    Capacity(const Device& d);

public:
    bool operator==(const Capacity& other) const {
        return other.m_Size == m_Size;
    }
    bool operator!=(const Capacity& other) const {
        return other.m_Size != m_Size;
    }
    bool operator>(const Capacity& other) const {
        return other.m_Size > m_Size;
    }
    bool operator<(const Capacity& other) const {
        return other.m_Size < m_Size;
    }
    bool operator>=(const Capacity& other) const {
        return other.m_Size >= m_Size;
    }
    bool operator<=(const Capacity& other) const {
        return other.m_Size <= m_Size;
    }

    qint64 toInt(Unit u) const;
    double toDouble(Unit u) const;

    bool isValid() const;

    static QString formatByteSize(double size, int precision = 2);
    static const QString& invalidString() {
        return m_InvalidString;    /**< @return string representing an invalid capacity */
    }
    static QString unitName(Unit u, qint64 val = 1);
    static qint64 unitFactor(Unit from, Unit to);

private:
    qint64 m_Size;
    static const QString m_InvalidString;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(Capacity::Flags)

#endif
