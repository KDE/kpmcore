/*
    SPDX-FileCopyrightText: 2008-2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2015 Teo Mrnjavac <teo@kde.org>
    SPDX-FileCopyrightText: 2016 Andrius Štikonas <andrius@stikonas.eu>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KPMCORE_SHREDFILESYSTEMJOB_H
#define KPMCORE_SHREDFILESYSTEMJOB_H

#include "jobs/job.h"

#include <QString>

class Partition;
class Device;
class Report;

/** Securely delete and shred a FileSystem.

    Shreds (overwrites with random data) a FileSystem on given Partition and Device.

    @author Volker Lanz <vl@fidra.de>
*/
class ShredFileSystemJob : public Job
{
public:
    ShredFileSystemJob(Device& d, Partition& p, bool randomShred);

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

private:
    Device& m_Device;
    Partition& m_Partition;
    bool m_RandomShred;
};

#endif
