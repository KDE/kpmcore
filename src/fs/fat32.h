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

#if !defined(KPMCORE_FAT32_H)

#define KPMCORE_FAT32_H

#include "util/libpartitionmanagerexport.h"

#include "fs/fat16.h"

#include <QtGlobal>

class Report;

class QString;

namespace FS
{
/** A fat32 file system.

    Basically the same as a fat16 file system.

    @author Volker Lanz <vl@fidra.de>
 */
class LIBKPMCORE_EXPORT fat32 : public fat16
{
public:
    fat32(qint64 firstsector, qint64 lastsector, qint64 sectorsused, const QString& label);

public:
    bool create(Report& report, const QString& deviceNode) override;
    bool updateUUID(Report& report, const QString& deviceNode) const override;

    qint64 minCapacity() const override;
    qint64 maxCapacity() const override;
};
}

#endif
