/*
    SPDX-FileCopyrightText: 2008-2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2016 Andrius Štikonas <andrius@stikonas.eu>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KPMCORE_BACKUPFILESYSTEMJOB_H
#define KPMCORE_BACKUPFILESYSTEMJOB_H

#include "jobs/job.h"

#include <QString>

class Partition;
class Device;
class Report;

/** Back up a FileSystem.

    Backs up a FileSystem from a given Device and Partition to a file with the given filename.

    @author Volker Lanz <vl@fidra.de>
*/
class BackupFileSystemJob : public Job
{
public:
    BackupFileSystemJob(Device& sourcedevice, Partition& sourcepartition, const QString& filename);

public:
    bool run(Report& parent) override;
    qint32 numSteps() const override;
    QString description() const override;

protected:
    Partition& sourcePartition() {
        return m_SourcePartition;
    }
    const Partition& sourcePartition() const {
        return m_SourcePartition;
    }

    Device& sourceDevice() {
        return m_SourceDevice;
    }
    const Device& sourceDevice() const {
        return m_SourceDevice;
    }

    const QString& fileName() const {
        return m_FileName;
    }

private:
    Device& m_SourceDevice;
    Partition& m_SourcePartition;
    QString m_FileName;
};

#endif
