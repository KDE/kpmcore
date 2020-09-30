/*
    SPDX-FileCopyrightText: 2008-2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2014-2018 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2015 Chris Campbell <c.j.campbell@ed.ac.uk>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KPMCORE_GLOBALLOG_H
#define KPMCORE_GLOBALLOG_H

#include "util/libpartitionmanagerexport.h"

#include <QString>
#include <QObject>
#include <QtGlobal>

class LIBKPMCORE_EXPORT Log
{
public:
    enum class Level {
        debug,
        information,
        warning,
        error,
    };

public:
    Log(Level lev = Level::information) : ref(1), level(lev) {}
    ~Log();
    Log(const Log& other) : ref(other.ref + 1), level(other.level) {}

private:
    quint32 ref;
    Level level;
};

/** Global logging.
    @author Volker Lanz <vl@fidra.de>
*/
class LIBKPMCORE_EXPORT GlobalLog : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(GlobalLog)

    friend class Log;
    friend Log operator<<(Log l, const QString& s);
    friend Log operator<<(Log l, qint64 i);

private:
    GlobalLog() : msg() {}

Q_SIGNALS:
    void newMessage(Log::Level, const QString&);

public:
    static GlobalLog* instance();

private:
    void append(const QString& s) {
        msg += s;
    }
    void flush(Log::Level level);

private:
    QString msg;
};

inline Log operator<<(Log l, const QString& s)
{
    GlobalLog::instance()->append(s);
    return l;
}

inline Log operator<<(Log l, qint64 i)
{
    GlobalLog::instance()->append(QString::number(i));
    return l;
}

#endif
