/*
    SPDX-FileCopyrightText: 2020 Gaël PORTAY <gael.portay@collabora.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/


#ifndef KPMCORE_SETPARTITIONUUIDJOB_H
#define KPMCORE_SETPARTITIONUUIDJOB_H

#include "jobs/job.h"

class Partition;
class Device;
class Report;

class QString;

/** Set a Partition UUID (GPT only).
    @author Gaël PORTAY <gael.portay@collabora.com>
*/
class SetPartitionUUIDJob : public Job
{
public:
    SetPartitionUUIDJob(Device& d, Partition& p, const QString& newUUID);

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

    Device& device() {
        return m_Device;
    }
    const Device& device() const {
        return m_Device;
    }

    const QString& uuid() const {
        return m_UUID;
    }
    void setUUID(const QString& u) {
        m_UUID = u;
    }

private:
    Device& m_Device;
    Partition& m_Partition;
    QString m_UUID;
};

#endif
