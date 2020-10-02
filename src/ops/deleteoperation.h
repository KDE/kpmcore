/*
    SPDX-FileCopyrightText: 2008-2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2014-2018 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2015 Chris Campbell <c.j.campbell@ed.ac.uk>
    SPDX-FileCopyrightText: 2015 Teo Mrnjavac <teo@kde.org>
    SPDX-FileCopyrightText: 2018 Caio Jordão Carvalho <caiojcarvalho@gmail.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KPMCORE_DELETEOPERATION_H
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
