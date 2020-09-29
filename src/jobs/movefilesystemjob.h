/*
    SPDX-FileCopyrightText: 2008-2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2016 Andrius Štikonas <andrius@stikonas.eu>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KPMCORE_MOVEFILESYSTEMJOB_H
#define KPMCORE_MOVEFILESYSTEMJOB_H

#include "jobs/job.h"

class Partition;
class Device;
class Report;

class QString;

/** Move a FileSystem.

    Moves a FileSystem on a given Device and Partition to a new start sector.

    @author Volker Lanz <vl@fidra.de>
*/
class MoveFileSystemJob : public Job
{
public:
    MoveFileSystemJob(Device& d, Partition& p, qint64 newstart);

public:
    bool run(Report& parent) override;
    qint32 numSteps() const override;
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

    qint64 newStart() const {
        return m_NewStart;
    }

private:
    Device& m_Device;
    Partition& m_Partition;
    qint64 m_NewStart;
};

#endif
