/*
    SPDX-FileCopyrightText: 2026 Ramil Nurmanov <ramil2004nur@gmail.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KPMCORE_TAKEOWNERSHIPOPERATION_H
#define KPMCORE_TAKEOWNERSHIPOPERATION_H

#include "util/libpartitionmanagerexport.h"

#include "fs/filesystem.h"
#include "ops/operation.h"

#include <QString>

class Partition;
class TakeOwnershipJob;

class LIBKPMCORE_EXPORT TakeOwnershipOperation : public Operation
{
    Q_DISABLE_COPY(TakeOwnershipOperation)

public:
    TakeOwnershipOperation(Partition &p, const QString &userName, bool recursive);

    static bool isCriticalMountPoint(const QString &mountPoint);
    static bool supportsOwnership(FileSystem::Type type);

public:
    QString iconName() const override { return QStringLiteral("user-properties"); }
    QString description() const override;
    void preview() override {}
    void undo() override {}

    bool targets(const Device &d) const override;
    bool targets(const Partition &p) const override;

protected:
    Partition &targetPartition() { return m_Partition; }
    const Partition &targetPartition() const { return m_Partition; }

private:
    Partition &m_Partition;
    QString m_UserName;
    bool m_Recursive;
    TakeOwnershipJob *m_Job;
};

#endif
