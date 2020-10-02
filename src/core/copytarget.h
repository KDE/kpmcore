/*
    SPDX-FileCopyrightText: 2008 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2008 Laurent Montel <montel@kde.org>
    SPDX-FileCopyrightText: 2016-2018 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2018 Huzaifa Faruqui <huzaifafaruqui@gmail.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KPMCORE_COPYTARGET_H
#define KPMCORE_COPYTARGET_H

#include <QtGlobal>

class QString;

/** Base class for something to copy to.

    Abstract base class for all copy targets. Used together with CopySource to
    implement moving, copying, restoring and backing up FileSystems.

    @see CopySource
    @author Volker Lanz <vl@fidra.de>
*/
class CopyTarget
{
    Q_DISABLE_COPY(CopyTarget)

protected:
    CopyTarget() : m_BytesWritten(0) {}
    virtual ~CopyTarget() {}

public:
    virtual bool open() = 0;
    virtual qint64 firstByte() const = 0;
    virtual qint64 lastByte() const = 0;
    virtual QString path() const = 0;
    qint64 bytesWritten() const {
        return m_BytesWritten;
    }

protected:
    void setBytesWritten(qint64 s) {
        m_BytesWritten = s;
    }

private:
    qint64 m_BytesWritten;
};

#endif
