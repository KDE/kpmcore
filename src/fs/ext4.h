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

#if !defined(KPMCORE_EXT4_H)

#define KPMCORE_EXT4_H

#include "util/libpartitionmanagerexport.h"

#include "fs/ext2.h"

#include <QtGlobal>

class Report;

class QString;

namespace FS
{
/** An ext4 file system.

    Basically the same as ext2.

    @author Volker Lanz <vl@fidra.de>
 */
class LIBKPMCORE_EXPORT ext4 : public ext2
{
public:
    ext4(qint64 firstsector, qint64 lastsector, qint64 sectorsused, const QString& label);

public:
    bool create(Report& report, const QString& deviceNode) override;
    bool resizeOnline(Report& report, const QString& deviceNode, const QString& mountPoint, qint64 length) const override;
    qint64 maxCapacity() const override;

    CommandSupportType supportGrowOnline() const override {
        return m_Grow;
    }
};
}

#endif
