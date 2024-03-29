/*
    SPDX-FileCopyrightText: 2008-2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2014 Andrius Štikonas <andrius@stikonas.eu>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "util/report.h"
#include "util/htmlreport.h"

#include "backend/corebackend.h"
#include "backend/corebackendmanager.h"

#include <KLocalizedString>

#include <sys/utsname.h>

/** Creates a new Report instance.
    @param p pointer to the parent instance. May be nullptr if this is a new root Report.
    @param cmd the command
*/
Report::Report(Report* p, const QString& cmd) :
    QObject(),
    m_Parent(p),
    m_Children(),
    m_Command(cmd),
    m_Output(),
    m_Status()
{
}

/** Destroys a Report instance and all its children. */
Report::~Report()
{
    qDeleteAll(children());
}

/** Creates a new child for this Report and appends it to its list of children.
    @param cmd the command
    @return pointer to a new Report child
*/
Report* Report::newChild(const QString& cmd)
{
    Report* r = new Report(this, cmd);
    m_Children.append(r);
    return r;
}

/**
    @return the Report converted to HTML
    @see toText()
*/
QString Report::toHtml() const
{
    QString s;

    if (parent() == root())
        s += QStringLiteral("<div>\n");
    else if (parent() != nullptr)
        s += QStringLiteral("<div style='margin-left:24px;margin-top:12px;margin-bottom:12px'>\n");

    if (!command().isEmpty())
        s += QStringLiteral("\n<b>") + command().toHtmlEscaped() + QStringLiteral("</b>\n\n");

    if (!output().isEmpty())
        s += QStringLiteral("<pre>") + output().toHtmlEscaped() + QStringLiteral("</pre>\n\n");

    if (children().size() == 0)
        s += QStringLiteral("<br/>\n");
    else
        for (const auto &child : children())
            s += child->toHtml();

    if (!status().isEmpty())
        s += QStringLiteral("<b>") + status().toHtmlEscaped() + QStringLiteral("</b><br/>\n\n");

    if (parent() != nullptr)
        s += QStringLiteral("</div>\n\n");

    return s;
}

/**
    @return the Report converted to plain text
    @see toHtml()
*/
QString Report::toText() const
{
    QString s;

    if (!command().isEmpty()) {
        s += QStringLiteral("==========================================================================================\n");
        s += command() + QStringLiteral("\n");
        s += QStringLiteral("==========================================================================================\n");
    }

    if (!output().isEmpty())
        s += output() + QStringLiteral("\n");

    for (const auto &child : children())
        s += child->toText();

    return s;
}

/** Adds a string to this Report's output.

    This is usually not what you want. In most cases, you will want to create a new child Report.

    @param s the string to add to the output

    @see newChild()
*/
void Report::addOutput(const QString& s)
{
    m_Output += s;
    root()->emitOutputChanged();
}

void Report::emitOutputChanged()
{
    Q_EMIT outputChanged();
}

/** @return the root Report */
Report* Report::root()
{
    Report* rval = this;

    while (rval->parent() != nullptr)
        rval = rval->parent();

    return rval;
}

/**
    @overload
*/
const Report* Report::root() const
{
    const Report* rval = this;

    while (rval->parent() != nullptr)
        rval = rval->parent();

    return rval;
}

/** @return a Report line to write to */
ReportLine Report::line()
{
    return ReportLine(*this);
}

#include "moc_report.cpp"
