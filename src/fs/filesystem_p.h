/*************************************************************************
 *  Copyright (C) 2018 by Andrius Štikonas <andrius@stikonas.eu>         *
 *                                                                       *
 *  This program is free software; you can redistribute it and/or        *
 *  modify it under the terms of the GNU General Public License as       *
 *  published by the Free Software Foundation; either version 3 of       *
 *  the License, or (at your option) any later version.                  *
 *                                                                       *
 *  This program is distributed in the hope that it will be useful,      *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of       *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        *
 *  GNU General Public License for more details.                         *
 *                                                                       *
 *  You should have received a copy of the GNU General Public License    *
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.*
 *************************************************************************/

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
