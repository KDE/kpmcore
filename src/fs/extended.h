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

#if !defined(KPMCORE_EXTENDED_H)

#define KPMCORE_EXTENDED_H

#include "util/libpartitionmanagerexport.h"

#include "fs/filesystem.h"

#include <QtGlobal>

class QString;

namespace FS
{
/** An extended file system.

    A FileSystem for an extended Partition. Of course, extended partitions do not actually have
    a file system, but we need this to be able to create, grow, shrink or move them.

    @author Volker Lanz <vl@fidra.de>
 */

class LIBKPMCORE_EXPORT extended : public FileSystem
{
public:
    extended(qint64 firstsector, qint64 lastsector, qint64 sectorsused, const QString& label);

public:

    bool create(Report&, const QString&) override;

    CommandSupportType supportCreate() const override {
        return m_Create;
    }
    CommandSupportType supportGrow() const override {
        return m_Grow;
    }
    CommandSupportType supportShrink() const override {
        return m_Shrink;
    }
    CommandSupportType supportMove() const override {
        return m_Move;
    }

    bool supportToolFound() const override {
        return true;
    }

public:
    static CommandSupportType m_Create;
    static CommandSupportType m_Grow;
    static CommandSupportType m_Shrink;
    static CommandSupportType m_Move;
};
}

#endif
