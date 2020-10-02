/*
    SPDX-FileCopyrightText: 2008 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2014-2018 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2015 Teo Mrnjavac <teo@kde.org>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "core/copysourceshred.h"

/** Constructs a CopySourceShred with the given @p size
    @param s the size the copy source will (pretend to) have
*/
CopySourceShred::CopySourceShred(qint64 s, bool randomShred) :
    CopySource(),
    m_Size(s),
    m_SourceFile(randomShred ? QStringLiteral("/dev/urandom") : QStringLiteral("/dev/zero"))
{
}

/** Opens the shred source.
    @return true on success
*/
bool CopySourceShred::open()
{
    return sourceFile().open(QIODevice::ReadOnly);
}

/** Returns the length of the source in bytes.
    @return length of the source in bytes.
*/
qint64 CopySourceShred::length() const
{
    return size();
}
