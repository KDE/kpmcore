/*
    SPDX-FileCopyrightText: 2008-2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2025 Eugene Shalygin <eugene.shalygin@gmail.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KPMCORE_SETPARTLABELOPERATION_H
#define KPMCORE_SETPARTLABELOPERATION_H

#include "core/partitiontable.h"
#include "ops/operation.h"
#include "util/libpartitionmanagerexport.h"

#include <QString>

class Device;
class OperationStack;
class Partition;

class SetPartitionLabelJob;

/** Set Partition label.

 Sets the Partition label for the given Partition on the given Device.

 @author Eugene Shalygin <eugene.shalygin@gmail.com>
*/
class LIBKPMCORE_EXPORT SetPartLabelOperation: public Operation
{
    friend class OperationStack;

    Q_DISABLE_COPY(SetPartLabelOperation)

public:
    SetPartLabelOperation(Device& d, Partition& p, const QString& label);

public:
    QString iconName() const override {
        return QStringLiteral("edit-rename");
    }

    QString description() const override;
    void preview() override;
    void undo() override;

    bool targets(const Device& d) const override;
    bool targets(const Partition& p) const override;

protected:
    Partition& partition() {
        return m_partition;
    }
    const Partition& partition() const {
        return m_partition;

    }

    Device& targetDevice() {
        return m_TargetDevice;
    }
    const Device& targetDevice() const {
        return m_TargetDevice;
    }

    const QString& oldLabel() const {
        return m_OldLabel;
    }
    const QString& newLabel() const {
        return m_NewLabel;
    }

    void setOldLabel(const QString& value) {
        m_OldLabel = value;
    }

    SetPartitionLabelJob* labelJob() {
        return m_LabelJob;
    }

private:
    Device& m_TargetDevice;
    Partition& m_partition;
    QString m_OldLabel;
    QString m_NewLabel;
    SetPartitionLabelJob* m_LabelJob;
};

#endif
