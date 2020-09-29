/*
    SPDX-FileCopyrightText: 2020 Gaël PORTAY <gael.portay@collabora.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KPMCORE_SETPARTITIONNAMEJOB_H
#define KPMCORE_SETPARTITIONNAMEJOB_H

#include "jobs/job.h"

class Partition;
class Device;
class Report;

class QString;

/** Set a Partition label (GPT only).
    @author Gaël PORTAY <gael.portay@collabora.com>
*/
class SetPartitionLabelJob : public Job
{
public:
    SetPartitionLabelJob(Device& d, Partition& p, const QString& newLabel);

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

    const QString& label() const {
        return m_Label;
    }
    void setLabel(const QString& l) {
        m_Label = l;
    }

private:
    Device& m_Device;
    Partition& m_Partition;
    QString m_Label;
};

#endif
