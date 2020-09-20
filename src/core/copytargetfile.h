/*
    SPDX-FileCopyrightText: 2008-2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2016-2018 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2019 Yuri Chornoivan <yurchor@ukr.net>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KPMCORE_COPYTARGETFILE_H
#define KPMCORE_COPYTARGETFILE_H

#include "core/copytarget.h"

#include <QtGlobal>
#include <QFile>

class QString;

/** A file to copy to.

    Repesents a target file to copy to. Used to back up a FileSystem to a file.

    @see CopySourceFile, CopyTargetDevice
    @author Volker Lanz <vl@fidra.de>
*/
class CopyTargetFile : public CopyTarget
{
public:
    explicit CopyTargetFile(const QString& filename);

public:
    bool open() override;

    qint64 firstByte() const override {
        return 0;    /**< @return always 0 for a file */
    }
    qint64 lastByte() const override {
        return bytesWritten();    /**< @return the number of bytes written so far */
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
