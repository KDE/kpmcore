/*
    SPDX-FileCopyrightText: 2008-2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2016 Andrius Štikonas <andrius@stikonas.eu>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KPMCORE_DELETEFILESYSTEMJOB_H
#define KPMCORE_DELETEFILESYSTEMJOB_H

#include "jobs/job.h"

class Partition;
class Device;
class Report;

class QString;

/** Delete a FileSystem.

    Delete and clobber the FileSystem on the given Partition on the given Device.

    @author Volker Lanz <vl@fidra.de>
*/
class DeleteFileSystemJob : public Job
{
public:
    DeleteFileSystemJob(Device& d, Partition& p);

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
};

#endif
