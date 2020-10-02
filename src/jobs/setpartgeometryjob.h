/*
    SPDX-FileCopyrightText: 2008-2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2016 Andrius Štikonas <andrius@stikonas.eu>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KPMCORE_SETPARTGEOMETRYJOB_H
#define KPMCORE_SETPARTGEOMETRYJOB_H

#include "jobs/job.h"

#include <QtGlobal>

class Partition;
class Device;
class Report;

class QString;

/** Set a Partition's geometry.

    Sets the geometry for a given Partition on a given Device to a new start sector and/or a new
    length. This does not move the FileSystem, it only updates the partition table entry for the
    Partition and is usually run together with MoveFileSystemJob or ResizeFileSystemJob for that reason.

    @author Volker Lanz <vl@fidra.de>
*/
class SetPartGeometryJob : public Job
{
public:
    SetPartGeometryJob(Device& d, Partition& p, qint64 newstart, qint64 newlength);

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

    qint64 newStart() const {
        return m_NewStart;
    }
    qint64 newLength() const {
        return m_NewLength;
    }

private:
    Device& m_Device;
    Partition& m_Partition;
    qint64 m_NewStart;
    qint64 m_NewLength;
};

#endif
