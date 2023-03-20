/*
    SPDX-FileCopyrightText: 2021 Tomaz Canabrava <tcanabrava@kde.org>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KPMCORE_CHANGEPERMISSIONJOB_H
#define KPMCORE_CHANGEPERMISSIONJOB_H

#include "jobs/job.h"

class Partition;
class Report;

class QString;

/** Check a FileSystem.
    @author Volker Lanz <vl@fidra.de>
*/
class ChangePermissionJob : public Job
{
public:
    /* Permission should be set in the partition. */
    explicit ChangePermissionJob(Partition& p);

public:
    bool run(Report& parent) override;
    QString description() const override;

protected:
    Partition& partition() {
        return m_Partition;
    }
    const Partition& partition() const {
        return m_Partition;
    }

private:
    Partition& m_Partition;
    QString m_permissions;
};

#endif
