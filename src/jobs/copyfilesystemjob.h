/*
    SPDX-FileCopyrightText: 2008-2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2016 Andrius Štikonas <andrius@stikonas.eu>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KPMCORE_COPYFILESYSTEMJOB_H
#define KPMCORE_COPYFILESYSTEMJOB_H

#include "jobs/job.h"

#include <QtGlobal>

class Partition;
class Device;
class Report;

class QString;

/** Copy a FileSystem.

    Copy a FileSystem on a given Partition and Device to another Partition on a (possibly other) Device.

    @author Volker Lanz <vl@fidra.de>
*/
class CopyFileSystemJob : public Job
{
public:
    CopyFileSystemJob(Device& targetdevice, Partition& targetpartition, Device& sourcedevice, Partition& sourcepartition);

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

private:
    Device& m_TargetDevice;
    Partition& m_TargetPartition;
    Device& m_SourceDevice;
    Partition& m_SourcePartition;
};

#endif
