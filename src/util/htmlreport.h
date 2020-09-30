/*
    SPDX-FileCopyrightText: 2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2015 Chris Campbell <c.j.campbell@ed.ac.uk>
    SPDX-FileCopyrightText: 2016 Andrius Štikonas <andrius@stikonas.eu>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KPMCORE_HTMLREPORT_H
#define KPMCORE_HTMLREPORT_H

#include "util/libpartitionmanagerexport.h"

class QString;

class LIBKPMCORE_EXPORT HtmlReport
{
public:
    HtmlReport() {}

public:
    static QString tableLine(const QString& label, const QString contents);
    QString header();
    QString footer();
};

#endif
