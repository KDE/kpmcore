/*
    SPDX-FileCopyrightText: 2008-2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2014-2016 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2015 Chris Campbell <c.j.campbell@ed.ac.uk>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KPMCORE_CREATEPARTITIONTABLEOPERATION_H
#define KPMCORE_CREATEPARTITIONTABLEOPERATION_H

#include "util/libpartitionmanagerexport.h"

#include "ops/operation.h"

#include "core/partitiontable.h"

#include <QString>

class Device;
class CreatePartitionTableJob;
class PartitionTable;
class OperationStack;

/** Create a PartitionTable.
    @author Volker Lanz <vl@fidra.de>
*/
class LIBKPMCORE_EXPORT CreatePartitionTableOperation : public Operation
{
    Q_DISABLE_COPY(CreatePartitionTableOperation)

    friend class OperationStack;

public:
    CreatePartitionTableOperation(Device& d, PartitionTable::TableType t);
    CreatePartitionTableOperation(Device& d, PartitionTable* ptable);
    ~CreatePartitionTableOperation();

public:
    QString iconName() const override {
        return QStringLiteral("edit-clear");
    }
    QString description() const override;
    void preview() override;
    void undo() override;
    bool execute(Report& parent) override;

    bool targets(const Device& d) const override;
    bool targets(const Partition&) const  override{
        return false;
    }

    static bool canCreate(const Device* device);

protected:
    Device& targetDevice() {
        return m_TargetDevice;
    }
    const Device& targetDevice() const {
        return m_TargetDevice;
    }

    PartitionTable* partitionTable() {
        return m_PartitionTable;
    }
    const PartitionTable* partitionTable() const {
        return m_PartitionTable;
    }

    PartitionTable* oldPartitionTable() {
        return m_OldPartitionTable;
    }
    void setOldPartitionTable(PartitionTable* old) {
        m_OldPartitionTable = old;
    }

    CreatePartitionTableJob* createPartitionTableJob() {
        return m_CreatePartitionTableJob;
    }

private:
    Device& m_TargetDevice;
    PartitionTable* m_OldPartitionTable;
    PartitionTable* m_PartitionTable;
    CreatePartitionTableJob* m_CreatePartitionTableJob;
};

#endif
