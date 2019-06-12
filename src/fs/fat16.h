/*************************************************************************
 *  Copyright (C) 2008,2009 by Volker Lanz <vl@fidra.de>                 *
 *  Copyright (C) 2017 by Andrius Štikonas <andrius@stikonas.eu>         *
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

#ifndef KPMCORE_FAT16_H
#define KPMCORE_FAT16_H

#include "fs/fat12.h"

class Report;

class QString;

namespace FS
{
/** A fat16 file system.
    @author Volker Lanz <vl@fidra.de>
 */
class LIBKPMCORE_EXPORT fat16 : public fat12
{
public:
    fat16(qint64 firstsector, qint64 lastsector, qint64 sectorsused, const QString& label);
    fat16(qint64 firstsector, qint64 lastsector, qint64 sectorsused, const QString& label, FileSystem::Type type);

public:
    void init() override;

    bool create(Report& report, const QString& deviceNode) override;
    bool resize(Report& report, const QString& deviceNode, qint64 length) const override;

    CommandSupportType supportGrow() const override {
        return m_Grow;
    }
    CommandSupportType supportShrink() const override {
        return m_Shrink;
    }

    qint64 minCapacity() const override;
    qint64 maxCapacity() const override;
    bool supportToolFound() const override;
};
}

#endif
