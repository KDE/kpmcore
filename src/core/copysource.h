/*
    SPDX-FileCopyrightText: 2008 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2008 Laurent Montel <montel@kde.org>
    SPDX-FileCopyrightText: 2016-2018 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2018 Huzaifa Faruqui <huzaifafaruqui@gmail.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KPMCORE_COPYSOURCE_H
#define KPMCORE_COPYSOURCE_H

#include <QtGlobal>

class CopyTarget;
class QString;

/** Base class for something to copy from.

    Abstract base class for all copy sources. Used in combination with CopyTarget
    to implement moving, copying, backing up and restoring FileSystems.

    @see CopyTarget
    @author Volker Lanz <vl@fidra.de>
*/
class CopySource
{
    Q_DISABLE_COPY(CopySource)

protected:
    CopySource() {}
    virtual ~CopySource() {}

public:
    virtual bool open() = 0;
    virtual QString path() const = 0;
    virtual qint64 length() const = 0;
    virtual bool overlaps(const CopyTarget& target) const = 0;

    virtual qint64 firstByte() const = 0;
    virtual qint64 lastByte() const = 0;
};

#endif
