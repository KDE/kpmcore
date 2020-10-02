/*
    SPDX-FileCopyrightText: 2008 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2014-2018 Andrius Štikonas <andrius@stikonas.eu>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "core/copysourcefile.h"

#include <QFile>
#include <QFileInfo>

/** Constructs a CopySourceFile from the given @p filename.
    @param filename filename of the file to copy from
*/
CopySourceFile::CopySourceFile(const QString& filename) :
    CopySource(),
    m_File(filename)
{
}

/** Opens the file.
    @return true on success
*/
bool CopySourceFile::open()
{
    return file().open(QIODevice::ReadOnly);
}

/** Returns the length of the file in bytes.
    @return length of the file in bytes.
*/
qint64 CopySourceFile::length() const
{
    return QFileInfo(file()).size();
}
