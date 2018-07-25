/*************************************************************************
 *  Copyright (C) 2008, 2010 by Volker Lanz <vl@fidra.de>                *
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

#if !defined(KPMCORE_DELETEOPERATION_H)

#define KPMCORE_DELETEOPERATION_H

#include "util/libpartitionmanagerexport.h"

#include "ops/operation.h"

#include <QString>

class Device;
class OperationStack;
class Partition;

class Job;
class DeletePartitionJob;

/** Delete a Partition.
    @author Volker Lanz <vl@fidra.de>
*/
class LIBKPMCORE_EXPORT DeleteOperation : public Operation
{
    friend class OperationStack;

    Q_DISABLE_COPY(DeleteOperation)

public:
    enum class ShredAction {
        NoShred,
        ZeroShred,
        RandomShred
    };

    DeleteOperation(Device& d, Partition* p, ShredAction shred = ShredAction::NoShred);
    ~DeleteOperation();

public:
    QString iconName() const override {
        return shredAction() == ShredAction::NoShred ?
               QStringLiteral("edit-delete") :
               QStringLiteral("edit-delete-shred");
    }
    QString description() const override;
    void preview() override;
    void undo() override;
    ShredAction shredAction() const {
        return m_ShredAction;
    }

    bool targets(const Device& d) const override;
    bool targets(const Partition& p) const override;

    static bool canDelete(const Partition* p);

protected:
    Device& targetDevice() {
        return m_TargetDevice;
    }
    const Device& targetDevice() const {
        return m_TargetDevice;
    }

    Partition& deletedPartition() {
        return *m_DeletedPartition;
    }
    const Partition& deletedPartition() const {
        return *m_DeletedPartition;
    }

    void checkAdjustLogicalNumbers(Partition& p, bool undo);

    void setDeletedPartition(Partition* p) {
        m_DeletedPartition = p;
    }

    Job* deleteFileSystemJob() {
        return m_DeleteFileSystemJob;
    }
    DeletePartitionJob* deletePartitionJob() {
        return m_DeletePartitionJob;
    }

private:
    Device& m_TargetDevice;
    Partition* m_DeletedPartition;
    ShredAction m_ShredAction;
    Job* m_DeleteFileSystemJob;
    DeletePartitionJob* m_DeletePartitionJob;
};

#endif
