 /*************************************************************************
 *  Copyright (C) 2019 by Shubham <aryan100jangid@gmail.com>             *
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
 
#ifndef KPMCORE_MINIX_H
#define KPMCORE_MINIX_H

#include "fs/filesystem.h"

#include "util/libpartitionmanagerexport.h"

class Report;

class QString;

namespace FS
{
/** A minix(Mini Unix) file system.
    @author Shubham <aryan100jangid@gmail.com>
 */    
class LIBKPMCORE_EXPORT minix : public FileSystem
{
public:
    minix(qint64 firstsector, qint64 lastsector, qint64 sectorsused, const QString& label, const QVariantMap& features = {});

    void init() override;
    
    bool check(Report& report, const QString&deviceNode) const override;
    bool create(Report& report, const QString&deviceNode) override;
    
    CommandSupportType supportGetLabel() const override {
        return m_GetLabel;
    }
    
    CommandSupportType supportGetUsed() const override {
        return m_GetUsed;
    }

    CommandSupportType supportShrink() const override {
        return m_Shrink;
    }

    CommandSupportType supportMove() const override {
        return m_Move;
    }
    
    CommandSupportType supportCheck() const override {
        return m_Check;
    }
    
    CommandSupportType supportCreate() const override {
        return m_Create;
    }
    
    CommandSupportType supportCopy() const override {
        return m_Copy;
    }
    
    CommandSupportType supportBackup() const override {
        return m_Backup;
    }

    qint64 maxCapacity() const override;
    int maxLabelLength() const override;
    SupportTool supportToolName() const override;
    bool supportToolFound() const override;

public:
    static CommandSupportType m_GetLabel;
    static CommandSupportType m_GetUsed;
    static CommandSupportType m_Shrink;
    static CommandSupportType m_Move;
    static CommandSupportType m_Create;
    static CommandSupportType m_Check;
    static CommandSupportType m_Copy;
    static CommandSupportType m_Backup;
};
}

#endif
