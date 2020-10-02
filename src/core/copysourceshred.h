/*
    SPDX-FileCopyrightText: 2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2015 Teo Mrnjavac <teo@kde.org>
    SPDX-FileCopyrightText: 2016-2018 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2018 Huzaifa Faruqui <huzaifafaruqui@gmail.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KPMCORE_COPYSOURCESHRED_H
#define KPMCORE_COPYSOURCESHRED_H

#include "core/copysource.h"

#include <QFile>

class CopyTarget;
class QString;

/** A source for securely overwriting a partition (shredding).

    Represents a source of data (random or zeros) to copy from. Used to securely overwrite data on disk.

    @author Volker Lanz <vl@fidra.de>
*/
class CopySourceShred : public CopySource
{
public:
    CopySourceShred(qint64 size, bool randomShred);

public:
    bool open() override;
    qint64 length() const override;

    bool overlaps(const CopyTarget&) const override {
        return false;    /**< @return false for shred source */
    }
    qint64 firstByte() const override {
        return 0;    /**< @return 0 for shred source */
    }
    qint64 lastByte() const override {
        return length();    /**< @return equal to length for shred source. @see length() */
    }
    QString path() const override {
        return m_SourceFile.fileName();
    }

protected:
    QFile& sourceFile() {
        return m_SourceFile;
    }
    const QFile& sourceFile() const {
        return m_SourceFile;
    }
    qint64 size() const {
        return m_Size;
    }

private:
    qint64 m_Size;
    QFile m_SourceFile;
};

#endif
