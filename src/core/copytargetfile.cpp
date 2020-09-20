/*
    SPDX-FileCopyrightText: 2008 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2014-2018 Andrius Štikonas <andrius@stikonas.eu>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "core/copytargetfile.h"

/** Constructs a file to write to.
    @param filename name of the file to write to
*/
CopyTargetFile::CopyTargetFile(const QString& filename) :
    CopyTarget(),
    m_File(filename)
{
}

/** Opens the file for writing.
    @return true on success
*/
bool CopyTargetFile::open()
{
    return file().open(QIODevice::WriteOnly | QIODevice::Truncate);
}
