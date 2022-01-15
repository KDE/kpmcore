/*
    SPDX-FileCopyrightText: 2020 Gaël PORTAY <gael.portay@collabora.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef SFDISKGPTATTRIBUTES__H
#define SFDISKGPTATTRIBUTES__H

#include <QtGlobal>
#include <QStringList>

/** Sfdisk GPT Attributes helpers.
    @author Gaël PORTAY <gael.portay@collabora.com>
*/
class SfdiskGptAttributes
{
public:
    static quint64 toULongLong(const QStringList& attrs);
    static QStringList toStringList(quint64 attrs);
};

#endif
