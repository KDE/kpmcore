/*
    SPDX-FileCopyrightText: 2008-2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2016 Andrius Štikonas <andrius@stikonas.eu>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KPMCORE_RESIZEFILESYSTEMJOB_H
#define KPMCORE_RESIZEFILESYSTEMJOB_H

#include "jobs/job.h"

class Partition;
class Device;
class Report;

class QString;

/** Resize a FileSystem.

    Resizes a FileSystem on a given Device and Partition to a new length. If the new length is -1, the
    FileSystem is maximized to fill the entire Partition.

    @author Volker Lanz <vl@fidra.de>
*/
class ResizeFileSystemJob : public Job
{
public:
    ResizeFileSystemJob(Device& d, Partition& p, qint64 newlength = -1);

public:
    bool run(Report& parent) override;
    qint32 numSteps() const override;
    QString description() const override;

protected:
    bool resizeFileSystemBackend(Report& report);

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

    qint64 newLength() const {
        return m_NewLength;
    }

    bool isMaximizing() const {
        return m_Maximize;
    }

private:
    Device& m_Device;
    Partition& m_Partition;
    bool m_Maximize;
    qint64 m_NewLength;
};

#endif
