/*
    SPDX-FileCopyrightText: 2008-2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2014-2016 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2015 Chris Campbell <c.j.campbell@ed.ac.uk>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KPMCORE_CHECKOPERATION_H
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
