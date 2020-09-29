/*
    SPDX-FileCopyrightText: 2008-2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2016 Andrius Štikonas <andrius@stikonas.eu>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KPMCORE_RESTOREFILESYSTEMJOB_H
#define KPMCORE_RESTOREFILESYSTEMJOB_H

#include "jobs/job.h"

#include <QString>

class Partition;
class Device;
class Report;

/** Restore a FileSystem.

    Restores a FileSystem from a file to a given Partition on a given Device.

    @author Volker Lanz <vl@fidra.de>
*/
class RestoreFileSystemJob : public Job
{
public:
    RestoreFileSystemJob(Device& targetdevice, Partition& targetpartition, const QString& filename);

public:
    bool run(Report& parent) override;
    qint32 numSteps() const override;
    QString description() const override;

protected:
    Partition& targetPartition() {
        return m_TargetPartition;
    }
    const Partition& targetPartition() const {
        return m_TargetPartition;
    }

    Device& targetDevice() {
        return m_TargetDevice;
    }
    const Device& targetDevice() const {
        return m_TargetDevice;
    }

    const QString& fileName() const {
        return m_FileName;
    }

private:
    Device& m_TargetDevice;
    Partition& m_TargetPartition;
    QString m_FileName;
};

#endif
