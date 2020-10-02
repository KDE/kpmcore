/*
    SPDX-FileCopyrightText: 2008-2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2014-2016 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2015 Chris Campbell <c.j.campbell@ed.ac.uk>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KPMCORE_CREATEFILESYSTEMOPERATION_H
#define KPMCORE_CREATEFILESYSTEMOPERATION_H

#include "util/libpartitionmanagerexport.h"

#include "ops/operation.h"

#include "fs/filesystem.h"

#include <QString>

class Partition;
class OperationStack;

class DeleteFileSystemJob;
class CreateFileSystemJob;
class CheckFileSystemJob;

/** Create a FileSystem.

    Creates a FileSystem on a given Partition and Device.

    @author Volker Lanz <vl@fidra.de>
*/
class LIBKPMCORE_EXPORT CreateFileSystemOperation : public Operation
{
    friend class OperationStack;

    Q_DISABLE_COPY(CreateFileSystemOperation)

public:
    CreateFileSystemOperation(Device& d, Partition& p, FileSystem::Type newType);
    ~CreateFileSystemOperation();

public:
    QString iconName() const override {
        return QStringLiteral("draw-eraser");
    }
    QString description() const override;
    void preview() override;
    void undo() override;
    bool execute(Report& parent) override;

    bool targets(const Device& d) const override;
    bool targets(const Partition& p) const override;

protected:
    Device& targetDevice() {
        return m_TargetDevice;
    }
    const Device& targetDevice() const {
        return m_TargetDevice;
    }

    Partition& partition() {
        return m_Partition;
    }
    const Partition& partition() const {
        return m_Partition;
    }

    FileSystem* newFileSystem() const {
        return m_NewFileSystem;
    }
    FileSystem* oldFileSystem() const {
        return m_OldFileSystem;
    }

    DeleteFileSystemJob* deleteJob() {
        return m_DeleteJob;
    }
    CreateFileSystemJob* createJob() {
        return m_CreateJob;
    }
    CheckFileSystemJob* checkJob() {
        return m_CheckJob;
    }

private:
    Device& m_TargetDevice;
    Partition& m_Partition;
    FileSystem* m_NewFileSystem;
    FileSystem* m_OldFileSystem;
    DeleteFileSystemJob* m_DeleteJob;
    CreateFileSystemJob* m_CreateJob;
    CheckFileSystemJob* m_CheckJob;
};

#endif
