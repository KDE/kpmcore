/*
    SPDX-FileCopyrightText: Tomaz Canabrava <tcanabrava@kde.org>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "jobs/changepermissionsjob.h"

#include "core/lvmdevice.h"
#include "core/partition.h"
#include "fs/luks.h"

#include "util/report.h"

#include <KLocalizedString>

/** Creates a new CreateVolumeGroupJob
 * @param permission the new permission for the partition, in chmod style.
 * @param partition the partition to change the permission.
 */
ChangePermissionJob::ChangePermissionJob(Partition& partition) :
    Job(),
    m_Partition(partition)
{
}

bool ChangePermissionJob::run(Report& parent)
{
    bool rval = false;

    auto &fs = m_Partition.fileSystem();

    Report* report = jobStarted(parent);

    if (m_Partition.roles().has(PartitionRole::Luks)) {
        auto &luksFs = static_cast<FS::luks&>(fs);
        rval = luksFs.execChangePosixPermission(*report, m_Partition.deviceNode());
    }
    else
        rval = fs.execChangePosixPermission(*report, m_Partition.deviceNode());

    jobFinished(*report, rval);

    return rval;
}

QString ChangePermissionJob::description() const
{
    return xi18nc("@info/plain", "Change the permissions of: <filename>%1</filename> to %2", m_Partition.deviceNode(), m_permissions);
}
