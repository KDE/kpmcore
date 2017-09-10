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

#if !defined(KPMCORE_UNKNOWN_H)

#define KPMCORE_UNKNOWN_H

#include "util/libpartitionmanagerexport.h"

#include "fs/filesystem.h"

#include <QtGlobal>

namespace FS
{
/** A pseudo file system for partitions whose file system we cannot determine.
    @author Volker Lanz <vl@fidra.de>
*/
class LIBKPMCORE_EXPORT unknown : public FileSystem
{
public:
    unknown(qint64 firstsector, qint64 lastsector, qint64 sectorsused, const QString& label);

public:
    bool supportToolFound() const override {
        return true;
    }
    bool canMount(const QString & deviceNode, const QString & mountPoint) const override;
};
}

#endif
