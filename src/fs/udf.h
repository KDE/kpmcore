/*************************************************************************
 *  Copyright (C) 2017 by Pali Rohár <pali.rohar@gmail.com>              *
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

#if !defined(UDF__H)

#define UDF__H

#include "util/libpartitionmanagerexport.h"

#include "fs/filesystem.h"

#include <QtGlobal>

class Report;

class QString;

namespace FS
{
/** A udf file system.
    @author Pali Rohár <pali.rohar@gmail.com>
 */
class LIBKPMCORE_EXPORT udf : public FileSystem
{
public:
    udf(qint64 firstsector, qint64 lastsector, qint64 sectorsused, const QString& label);

public:
    void init() override;

    bool create(Report& report, const QString& deviceNode) override;
    bool createWithLabel(Report& report, const QString& deviceNode, const QString& label) override;

    CommandSupportType supportGetLabel() const override {
        return cmdSupportCore;
    }
    CommandSupportType supportCreate() const override {
        return m_Create;
    }
    CommandSupportType supportCreateWithLabel() const override {
        return m_Create;
    }
    CommandSupportType supportMove() const override {
        return cmdSupportCore;
    }
    CommandSupportType supportCopy() const override {
        return cmdSupportCore;
    }
    CommandSupportType supportBackup() const override {
        return cmdSupportCore;
    }
    CommandSupportType supportGetUUID() const override {
        return cmdSupportCore;
    }

    qint64 minCapacity() const override;
    qint64 maxCapacity() const override;
    qint64 maxLabelLength() const override;
    QValidator* labelValidator(QObject *parent = nullptr) const override;
    SupportTool supportToolName() const override;
    bool supportToolFound() const override;

public:
    static CommandSupportType m_Create;
    static QRegularExpressionValidator m_LabelValidator;
};
}

#endif
