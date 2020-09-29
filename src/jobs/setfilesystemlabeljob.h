/*
    SPDX-FileCopyrightText: 2008-2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2016 Andrius Štikonas <andrius@stikonas.eu>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KPMCORE_SETFILESYSTEMLABELJOB_H
#define KPMCORE_SETFILESYSTEMLABELJOB_H

#include "jobs/job.h"

#include <QString>

class Partition;
class Report;
class OperationStack;

/** Set a FileSystem label.
    @author Volker Lanz <vl@fidra.de>
*/
class SetFileSystemLabelJob : public Job
{
    friend class OperationStack;

public:
    SetFileSystemLabelJob(Partition& p, const QString& newlabel);

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

    const QString& label() const {
        return m_Label;
    }
    void setLabel(const QString& l) {
        m_Label = l;
    }

private:
    Partition& m_Partition;
    QString m_Label;
};

#endif
