/*
    SPDX-FileCopyrightText: 2008-2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2016-2017 Andrius Štikonas <andrius@stikonas.eu>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KPMCORE_CREATEFILESYSTEMJOB_H
#define KPMCORE_CREATEFILESYSTEMJOB_H

#include "jobs/job.h"

class Device;
class Partition;
class Report;

class QString;

/** Create a FileSystem.
    @author Volker Lanz <vl@fidra.de>
*/
class CreateFileSystemJob : public Job
{
public:
    CreateFileSystemJob(Device& d, Partition& p, const QString& label = {});

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

private:
    Device& m_Device;
    Partition& m_Partition;
    const QString& m_Label;
};

#endif
