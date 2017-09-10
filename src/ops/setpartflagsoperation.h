/*************************************************************************
 *  Copyright (C) 2008 by Volker Lanz <vl@fidra.de>                      *
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

#if !defined(KPMCORE_SETPARTFLAGSOPERATION_H)

#define KPMCORE_SETPARTFLAGSOPERATION_H

#include "util/libpartitionmanagerexport.h"

#include "ops/operation.h"

#include "core/partitiontable.h"

#include <QString>

class Device;
class OperationStack;
class Partition;

class SetPartFlagsJob;

/** Set Partition flags.

    Sets the Partition flags for the given Partition on the given Device.

    @author Volker Lanz <vl@fidra.de>
*/
class LIBKPMCORE_EXPORT SetPartFlagsOperation : public Operation
{
    friend class OperationStack;

    Q_DISABLE_COPY(SetPartFlagsOperation)

public:
    SetPartFlagsOperation(Device& d, Partition& p, const PartitionTable::Flags& flags);

public:
    QString iconName() const override {
        return QStringLiteral("flag-blue");
    }
    QString description() const override;
    void preview() override;
    void undo() override;

    bool targets(const Device& d) const override;
    bool targets(const Partition& p) const override;

protected:
    Partition& flagPartition() {
        return m_FlagPartition;
    }
    const Partition& flagPartition() const {
        return m_FlagPartition;
    }

    Device& targetDevice() {
        return m_TargetDevice;
    }
    const Device& targetDevice() const {
        return m_TargetDevice;
    }

    const PartitionTable::Flags& oldFlags() const {
        return m_OldFlags;
    }
    const PartitionTable::Flags& newFlags() const {
        return m_NewFlags;
    }

    void setOldFlags(PartitionTable::Flags f) {
        m_OldFlags = f;
    }

    SetPartFlagsJob* flagsJob() {
        return m_FlagsJob;
    }

private:
    Device& m_TargetDevice;
    Partition& m_FlagPartition;
    PartitionTable::Flags m_OldFlags;
    PartitionTable::Flags m_NewFlags;
    SetPartFlagsJob* m_FlagsJob;
};

#endif
