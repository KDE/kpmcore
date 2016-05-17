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

#if !defined(HFSPLUS__H)

#define HFSPLUS__H

#include "util/libpartitionmanagerexport.h"

#include "fs/filesystem.h"

#include <QtGlobal>

class Report;

class QString;

namespace FS
{
/** An hfsplus file system.
    @author Volker Lanz <vl@fidra.de>
 */
class LIBKPMCORE_EXPORT hfsplus : public FileSystem
{
public:
    hfsplus(qint64 firstsector, qint64 lastsector, qint64 sectorsused, const QString& label);

public:
    virtual void init() override;

    virtual bool check(Report& report, const QString& deviceNode) const override;

    virtual CommandSupportType supportGetUsed() const override {
        return m_GetUsed;
    }
    virtual CommandSupportType supportShrink() const override {
        return m_Shrink;
    }
    virtual CommandSupportType supportMove() const override {
        return m_Move;
    }
    virtual CommandSupportType supportCheck() const override {
        return m_Check;
    }
    virtual CommandSupportType supportCopy() const override {
        return m_Copy;
    }
    virtual CommandSupportType supportBackup() const override {
        return m_Backup;
    }

    virtual qint64 maxCapacity() const override;
    virtual qint64 maxLabelLength() const override;
    virtual SupportTool supportToolName() const override;
    virtual bool supportToolFound() const override;

public:
    static CommandSupportType m_GetUsed;
    static CommandSupportType m_Shrink;
    static CommandSupportType m_Move;
    static CommandSupportType m_Check;
    static CommandSupportType m_Copy;
    static CommandSupportType m_Backup;
};
}

#endif
