/*************************************************************************
 *  Copyright (C) 2008 by Volker Lanz <vl@fidra.de>                      *
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

#if !defined(KPMCORE_UFS_H)

#define KPMCORE_UFS_H

#include "util/libpartitionmanagerexport.h"

#include "fs/filesystem.h"

#include <QtGlobal>

class QString;

namespace FS
{
/** A UFS file system.
    @author Volker Lanz <vl@fidra.de>
 */
class LIBKPMCORE_EXPORT ufs : public FileSystem
{
public:
    ufs(qint64 firstsector, qint64 lastsector, qint64 sectorsused, const QString& label);

public:
    CommandSupportType supportMove() const override {
        return m_Move;
    }
    CommandSupportType supportCopy() const override {
        return m_Copy;
    }
    CommandSupportType supportBackup() const override {
        return m_Backup;
    }

    bool supportToolFound() const override {
        return true;
    }

public:
    static CommandSupportType m_Move;
    static CommandSupportType m_Copy;
    static CommandSupportType m_Backup;
};
}

#endif
