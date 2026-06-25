/*
    SPDX-FileCopyrightText: 2026 Ramil Nurmanov <ramil2004nur@gmail.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "ops/takeownershipoperation.h"

#include "core/partition.h"
#include "core/device.h"

#include "jobs/takeownershipjob.h"

#include <KLocalizedString>
#include <QStringList>

bool TakeOwnershipOperation::supportsOwnership(FileSystem::Type type)
{
    switch (type)
    {
    case FileSystem::Type::Ext2:
    case FileSystem::Type::Ext3:
    case FileSystem::Type::Ext4:
    case FileSystem::Type::Btrfs:
    case FileSystem::Type::Xfs:
    case FileSystem::Type::Jfs:
    case FileSystem::Type::ReiserFS:
    case FileSystem::Type::Reiser4:
    case FileSystem::Type::Nilfs2:
    case FileSystem::Type::F2fs:
    case FileSystem::Type::Minix:
    case FileSystem::Type::Bcachefs:
    case FileSystem::Type::Ocfs2:
    case FileSystem::Type::Ufs:
        return true;
    default:
        return false;
    }
}

bool TakeOwnershipOperation::isCriticalMountPoint(const QString &mountPoint)
{
    static const QStringList criticalMountPoints = {
        QStringLiteral("/"),
        QStringLiteral("/boot"),
        QStringLiteral("/boot/efi"),
        QStringLiteral("/etc"),
        QStringLiteral("/usr"),
        QStringLiteral("/var"),
    };
    return criticalMountPoints.contains(mountPoint);
}

TakeOwnershipOperation::TakeOwnershipOperation(Partition &p, const QString &userName, bool recursive)
    : Operation(), m_Partition(p), m_UserName(userName), m_Recursive(recursive), m_Job(new TakeOwnershipJob(p, userName, recursive))
{
    addJob(m_Job);
}

bool TakeOwnershipOperation::targets(const Device &d) const
{
    return m_Partition.devicePath() == d.deviceNode();
}

bool TakeOwnershipOperation::targets(const Partition &p) const
{
    return p == m_Partition;
}

QString TakeOwnershipOperation::description() const
{
    if (m_Recursive)
        return xi18nc("@info:status", "Take ownership of <filename>%1</filename> and its contents for user %2",
                      m_Partition.mountPoint(), m_UserName);
    return xi18nc("@info:status", "Take ownership of <filename>%1</filename> for user %2",
                  m_Partition.mountPoint(), m_UserName);
}
