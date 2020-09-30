/*
    SPDX-FileCopyrightText: 2008-2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2012-2018 Andrius Štikonas <andrius@stikonas.eu>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "util/capacity.h"

#include "core/partition.h"
#include "core/device.h"

#include <KFormat>
#include <KLocalizedString>

#include <QDebug>

const QString Capacity::m_InvalidString = QStringLiteral("---");

/** Creates a new Capacity instance.
    @param size the size in bytes
*/
Capacity::Capacity(qint64 size) :
    m_Size(size)
{
}

/** Creates a new Capacity instance.
    @param p the Partition
    @param t type of Capacity
*/
Capacity::Capacity(const Partition& p, Type t) :
    m_Size(-1)
{
    switch (t) {
    case Type::Used: m_Size = p.used(); break;
    case Type::Available: m_Size = p.available(); break;
    case Type::Total: m_Size = p.capacity();
    }
}

/** Creates a new Capacity instance.
    @param d the Device
*/
Capacity::Capacity(const Device& d) :
    m_Size(d.capacity())
{
}

/** Returns the Capacity as qint64 converted to the given Unit.
    @param u the Unit to use
    @return the Capacity in the given Unit as qint64
*/
qint64 Capacity::toInt(Unit u) const
{
    return static_cast<qint64>(m_Size / unitFactor(Unit::Byte, u));
}

/** Returns the Capacity as double converted to the given Unit.
    @param u the Unit to use
    @return the Capacity in the given Unit as double
*/
double Capacity::toDouble(Unit u) const
{
    return static_cast<double>(m_Size) / unitFactor(Unit::Byte, u);
}

/** Returns a factor to convert between two Units.
    @param from the Unit to convert from
    @param to the Unit to convert to
    @return the factor to use for conversion
*/
qint64 Capacity::unitFactor(Unit from, Unit to)
{
    Q_ASSERT(from <= to);

    if (from > to) {
        qWarning() << "from: " << static_cast<uint>(from) << ", to: " << static_cast<uint>(to);
        return 1;
    }

    qint64 result = 1;

    qint32 a = static_cast<uint>(from);
    qint32 b = static_cast<uint>(to);

    while (b-- > a)
        result *= 1024;

    return result;
}

/** Returns the name of a given Unit.
    @param u the Unit to find the name for
    @return the name
*/
QString Capacity::unitName(Unit u, qint64 val)
{
    static QString unitNames[] = {
        xi18ncp("@item:intext unit", "Byte", "Bytes", val),
        xi18nc("@item:intext unit", "KiB"),
        xi18nc("@item:intext unit", "MiB"),
        xi18nc("@item:intext unit", "GiB"),
        xi18nc("@item:intext unit", "TiB"),
        xi18nc("@item:intext unit", "PiB"),
        xi18nc("@item:intext unit", "EiB"),
        xi18nc("@item:intext unit", "ZiB"),
        xi18nc("@item:intext unit", "YiB")
    };

    if (static_cast<quint32>(u) >= sizeof(unitNames) / sizeof(unitNames[0]))
        return xi18nc("@item:intext unit", "(unknown unit)");

    return unitNames[static_cast<quint32>(u)];
}

/** Determine if the capacity is valid.
    @return true if it is valid
*/
bool Capacity::isValid() const
{
    return m_Size >= 0;
}

QString Capacity::formatByteSize(double size, int precision)
{
    if (size < 0)
        return invalidString();
    return KFormat().formatByteSize(size, precision);
}
