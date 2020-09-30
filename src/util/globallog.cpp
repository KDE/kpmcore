/*
    SPDX-FileCopyrightText: 2008-2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2015 Teo Mrnjavac <teo@kde.org>
    SPDX-FileCopyrightText: 2020 Andrius Štikonas <andrius@stikonas.eu>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "util/globallog.h"

GlobalLog* GlobalLog::instance()
{
    static GlobalLog* p = nullptr;

    if (p == nullptr)
        p = new GlobalLog();

    return p;
}

void GlobalLog::flush(Log::Level lev)
{
    Q_EMIT newMessage(lev, msg);
    msg.clear();
}

// --------------------------------------------------------------------------

Log::~Log()
{
    if (--ref == 0)
        GlobalLog::instance()->flush(level);
}
