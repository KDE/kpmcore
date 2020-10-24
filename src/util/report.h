/*
    SPDX-FileCopyrightText: 2008-2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2008 Laurent Montel <montel@kde.org>
    SPDX-FileCopyrightText: 2015 Chris Campbell <c.j.campbell@ed.ac.uk>
    SPDX-FileCopyrightText: 2015 Teo Mrnjavac <teo@kde.org>
    SPDX-FileCopyrightText: 2016 Andrius Štikonas <andrius@stikonas.eu>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KPMCORE_REPORT_H
#define KPMCORE_REPORT_H

#include "util/libpartitionmanagerexport.h"

#include <QObject>
#include <QList>
#include <QString>
#include <QtGlobal>

class ReportLine;

/** Report details about running Operations and Jobs.

    Gather information for the report shown in the ProgressDialog's detail view.

    @author Volker Lanz <vl@fidra.de>
*/
class LIBKPMCORE_EXPORT Report : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(Report)

    friend Report& operator<<(Report& report, const QString& s);
    friend Report& operator<<(Report& report, qint64 i);

public:
    explicit Report(Report* p, const QString& cmd = QString());
    ~Report() override;

Q_SIGNALS:
    void outputChanged();

public:
    Report* newChild(const QString& cmd = QString());

    const QList<Report*>& children() const {
        return m_Children;    /**< @return the list of this Report's children */
    }

    Report* parent() {
        return m_Parent;    /**< @return pointer to this Reports parent. May be nullptr if this is the root Report */
    }
    const Report* parent() const {
        return m_Parent;    /**< @return pointer to this Reports parent. May be nullptr if this is the root Report */
    }

    Report* root();
    const Report* root() const;

    const QString& command() const {
        return m_Command;    /**< @return the command */
    }
    const QString& output() const {
        return m_Output;    /**< @return the output */
    }
    const QString& status() const {
        return m_Status;    /**< @return the status line */
    }

    void setCommand(const QString& s) {
        m_Command = s;    /**< @param s the new command */
    }
    void setStatus(const QString& s) {
        m_Status = s;    /**< @param s the new status */
    }
    void addOutput(const QString& s);

    QString toHtml() const;
    QString toText() const;

    ReportLine line();

    static QString htmlHeader();
    static QString htmlFooter();

protected:
    void emitOutputChanged();

private:
    Report* m_Parent;
    QList<Report*> m_Children;
    QString m_Command;
    QString m_Output;
    QString m_Status;
};

inline Report& operator<<(Report& report, const QString& s)
{
    report.addOutput(s);
    return report;
}

inline Report& operator<<(Report& report, qint64 i)
{
    report.addOutput(QString::number(i));
    return report;
}

class ReportLine
{
    friend ReportLine operator<<(ReportLine reportLine, const QString& s);
    friend ReportLine operator<<(ReportLine reportLine, qint64 i);
    friend class Report;

protected:
    ReportLine(Report& r) : ref(1), report(r.newChild()) {}

public:
    ~ReportLine() {
        if (--ref == 0) *report << QStringLiteral("\n");
    }
    ReportLine(const ReportLine& other) : ref(other.ref + 1), report(other.report) {}

private:
    ReportLine& operator=(const ReportLine&);

private:
    qint32 ref;
    Report* report;
};

inline ReportLine operator<<(ReportLine reportLine, const QString& s)
{
    *reportLine.report << s;
    return reportLine;
}

inline ReportLine operator<<(ReportLine reportLine, qint64 i)
{
    *reportLine.report << i;
    return reportLine;
}

#endif
