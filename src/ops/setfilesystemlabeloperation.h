/*
    SPDX-FileCopyrightText: 2008-2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2014-2016 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2015 Chris Campbell <c.j.campbell@ed.ac.uk>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KPMCORE_SETFILESYSTEMLABELOPERATION_H
#define KPMCORE_SETFILESYSTEMLABELOPERATION_H

#include "util/libpartitionmanagerexport.h"

#include "ops/operation.h"

#include <QString>

class OperationStack;
class Partition;

class SetFileSystemLabelJob;

/** Set a FileSystem label.

    Sets the FileSystem label for the given Partition.

    @author Volker Lanz <vl@fidra.de>
*/
class LIBKPMCORE_EXPORT SetFileSystemLabelOperation : public Operation
{
    friend class OperationStack;

    Q_DISABLE_COPY(SetFileSystemLabelOperation)

public:
    SetFileSystemLabelOperation(Partition& p, const QString& newlabel);

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
    Partition& labeledPartition() {
        return m_LabeledPartition;
    }
    const Partition& labeledPartition() const {
        return m_LabeledPartition;
    }

    const QString& oldLabel() const {
        return m_OldLabel;
    }
    const QString& newLabel() const {
        return m_NewLabel;
    }

    void setOldLabel(const QString& l) {
        m_OldLabel = l;
    }

    SetFileSystemLabelJob* labelJob() {
        return m_LabelJob;
    }

private:
    Partition& m_LabeledPartition;
    QString m_OldLabel;
    QString m_NewLabel;
    SetFileSystemLabelJob* m_LabelJob;
};

#endif
