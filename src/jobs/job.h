/*
    SPDX-FileCopyrightText: 2008-2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2016-2018 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2018 Huzaifa Faruqui <huzaifafaruqui@gmail.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KPMCORE_JOB_H
#define KPMCORE_JOB_H

#include "fs/filesystem.h"

#include "util/libpartitionmanagerexport.h"

#include <QObject>
#include <QtGlobal>

class QString;
class QIcon;

class CopySource;
class CopyTarget;
class Report;

/** Base class for all Jobs.

    Each Operation is made up of one or more Jobs. Usually, an Operation will run each Job it is
    made up of and only complete successfully if each Job could be run without error. Jobs are
    all-or-nothing and try to be as atomic as possible: A Job is either successfully run or not, there
    is no case where a Job finishes with a warning.

    @author Volker Lanz <vl@fidra.de>
*/
class LIBKPMCORE_EXPORT Job : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(Job)

public:
    /** Status of this Job */
    enum class Status : int {
        Pending,        /**< Pending, not yet run */
        Success,        /**< Successfully run */
        Error           /**< Running generated an error */
    };

protected:
    Job();

public:
    ~Job() override {}

Q_SIGNALS:
    void started();
    void progress(int);
    void finished();

public:
    virtual qint32 numSteps() const {
        return 1;    /**< @return the number of steps the job takes to complete */
    }
    virtual QString description() const = 0; /**< @return the Job's description */
    virtual bool run(Report& parent) = 0; /**< @param parent parent Report to add new child to for this Job @return true if successfully run */

    virtual QString statusIcon() const;
    virtual QString statusText() const;

    Status status() const {
        return m_Status;    /**< @return the Job's current status */
    }

    void emitProgress(int i);
    void updateReport(const QString& report);

protected:
    bool copyBlocks(Report& report, CopyTarget& target, CopySource& source);
    bool rollbackCopyBlocks(Report& report, CopyTarget& origTarget, CopySource& origSource);

    Report* jobStarted(Report& parent);
    void jobFinished(Report& report, bool b);

    void setStatus(Status s) {
        m_Status = s;
    }

private:
    Report *m_Report;
    Status m_Status;
};

#endif
