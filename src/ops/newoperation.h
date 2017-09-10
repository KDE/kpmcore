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

#if !defined(KPMCORE_NEWOPERATION_H)

#define KPMCORE_NEWOPERATION_H

#include "fs/filesystem.h"
#include "ops/operation.h"
#include "util/libpartitionmanagerexport.h"

#include <QString>

class Device;
class OperationStack;

class CreatePartitionJob;
class CreateFileSystemJob;
class SetFileSystemLabelJob;
class SetPartFlagsJob;
class CheckFileSystemJob;

/** Create a Partition.

    Creates the given Partition on the given Device.

    @author Volker Lanz <vl@fidra.de>
*/
class LIBKPMCORE_EXPORT NewOperation : public Operation
{
    friend class OperationStack;

    Q_DISABLE_COPY(NewOperation)

public:
    NewOperation(Device& d, Partition* p);
    ~NewOperation();

public:
    QString iconName() const override {
        return QStringLiteral("document-new");
    }
    QString description() const override;
    void preview() override;
    void undo() override;

    bool targets(const Device& d) const override;
    bool targets(const Partition& p) const override;

    static bool canCreateNew(const Partition* p);
    static Partition* createNew(const Partition& cloneFrom, FileSystem::Type type);

protected:
    Partition& newPartition() {
        return *m_NewPartition;
    }
    const Partition& newPartition() const {
        return *m_NewPartition;
    }

    Device& targetDevice() {
        return m_TargetDevice;
    }
    const Device& targetDevice() const {
        return m_TargetDevice;
    }

    CreatePartitionJob* createPartitionJob() {
        return m_CreatePartitionJob;
    }
    CreateFileSystemJob* createFileSystemJob() {
        return m_CreateFileSystemJob;
    }
    SetPartFlagsJob* setPartFlagsJob() {
        return m_SetPartFlagsJob;
    }
    SetFileSystemLabelJob* setLabelJob() {
        return m_SetFileSystemLabelJob;
    }
    CheckFileSystemJob* checkJob() {
        return m_CheckFileSystemJob;
    }

private:
    Device& m_TargetDevice;
    Partition* m_NewPartition;
    CreatePartitionJob* m_CreatePartitionJob;
    CreateFileSystemJob* m_CreateFileSystemJob;
    SetPartFlagsJob* m_SetPartFlagsJob;
    SetFileSystemLabelJob* m_SetFileSystemLabelJob;
    CheckFileSystemJob* m_CheckFileSystemJob;
};

#endif
