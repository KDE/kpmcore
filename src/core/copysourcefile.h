/*
    SPDX-FileCopyrightText: 2008-2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2016-2018 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2019 Yuri Chornoivan <yurchor@ukr.net>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KPMCORE_COPYSOURCEFILE_H
#define KPMCORE_COPYSOURCEFILE_H

#include "core/copysource.h"

#include <QtGlobal>
#include <QFile>

class QString;
class CopyTarget;

/** A file to copy from.

    Represents a file to copy from. Used to restore a FileSystem from a backup file.

    @author Volker Lanz <vl@fidra.de>
*/
class CopySourceFile : public CopySource
{
public:
    explicit CopySourceFile(const QString& filename);

public:
    bool open() override;
    qint64 length() const override;

    bool overlaps(const CopyTarget&) const override {
        return false;    /**< @return false for file */
    }
    qint64 firstByte() const override {
        return 0;    /**< @return 0 for file */
    }
    qint64 lastByte() const override {
        return length();    /**< @return equal to length for file. @see length() */
    }
    QString path() const override {
        return m_File.fileName();
    }

protected:
    QFile& file() {
        return m_File;
    }
    const QFile& file() const {
        return m_File;
    }

protected:
    QFile m_File;
};

#endif
