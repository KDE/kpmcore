/*
    SPDX-FileCopyrightText: 2018 Andrius Štikonas <andrius@stikonas.eu>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KPMCORE_FILESYSTEM_P_H
#define KPMCORE_FILESYSTEM_P_H

#include "fs/filesystem.h"

#include <QString>

class FileSystemPrivate {
public:
    FileSystem::Type m_Type;
    qint64 m_FirstSector;
    qint64 m_LastSector;
    qint64 m_SectorSize;
    qint64 m_SectorsUsed;
    QString m_Label;
    QString m_UUID;
};

#endif
