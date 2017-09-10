/*************************************************************************
 *  Copyright (C) 2008, 2011 by Volker Lanz <vl@fidra.de>                *
 *  Copyright (C) 2016 by Andrius Štikonas <andrius@stikonas.eu>         *
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

#if !defined(KPMCORE_CHECKOPERATION_H)

#define KPMCORE_CHECKOPERATION_H

#include "util/libpartitionmanagerexport.h"

#include "ops/operation.h"

#include <QString>

class Partition;
class Device;
class CheckFileSystemJob;
class ResizeFileSystemJob;

/** Check a Partition.
    @author Volker Lanz <vl@fidra.de>
*/
class LIBKPMCORE_EXPORT CheckOperation : public Operation
{
    friend class OperationStack;

    Q_DISABLE_COPY(CheckOperation)

public:
    CheckOperation(Device& targetDevice, Partition& checkedPartition);

public:
    QString iconName() const override {
        return QStringLiteral("flag");
    }
    QString description() const override;
    void preview() override {}
    void undo() override {}

    bool targets(const Device& d) const override;
    bool targets(const Partition& p) const override;

    static bool canCheck(const Partition* p);

protected:
    Device& targetDevice() {
        return m_TargetDevice;
    }
    const Device& targetDevice() const {
        return m_TargetDevice;
    }

    Partition& checkedPartition() {
        return m_CheckedPartition;
    }
    const Partition& checkedPartition() const {
        return m_CheckedPartition;
    }

    CheckFileSystemJob* checkJob() {
        return m_CheckJob;
    }
    ResizeFileSystemJob* maximizeJob() {
        return m_MaximizeJob;
    }

private:
    Device& m_TargetDevice;
    Partition& m_CheckedPartition;
    CheckFileSystemJob* m_CheckJob;
    ResizeFileSystemJob* m_MaximizeJob;
};

#endif
