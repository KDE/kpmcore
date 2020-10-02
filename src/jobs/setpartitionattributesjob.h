/*
    SPDX-FileCopyrightText: 2020 Gaël PORTAY <gael.portay@collabora.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KPMCORE_SETPARTITIONATTRIBUTESJOB_H
#define KPMCORE_SETPARTITIONATTRIBUTESJOB_H

#include "jobs/job.h"

class Partition;
class Device;
class Report;

/** Set a Partition attributes (GPT only).
    @author Gaël PORTAY <gael.portay@collabora.com>
*/
class SetPartitionAttributesJob : public Job
{
public:
    SetPartitionAttributesJob(Device& d, Partition& p, quint64 newAttrs);

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

    quint64 attributes() const {
        return m_Attributes;
    }
    void setAttributes(quint64 f) {
        m_Attributes = f;
    }

private:
    Device& m_Device;
    Partition& m_Partition;
    quint64 m_Attributes;
};

#endif
