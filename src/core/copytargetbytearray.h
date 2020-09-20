/*
    SPDX-FileCopyrightText: 2018 Andrius Štikonas <andrius@stikonas.eu>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KPMCORE_COPYTARGETBYTEARRAY_H
#define KPMCORE_COPYTARGETBYTEARRAY_H

#include "core/copytarget.h"

#include <QtGlobal>
#include <QByteArray>
#include <QString>

/** A file to copy to.

    Represents a target file to copy to. Used to back up a FileSystem to a file.

    @see CopySourceFile, CopyTargetDevice
    @author Volker Lanz <vl@fidra.de>
*/
class CopyTargetByteArray : public CopyTarget
{
public:
    explicit CopyTargetByteArray(QByteArray& array);

public:
    bool open() override {
        return true;
    }

    QString path() const override {
        return QString();
    }

    qint64 firstByte() const override {
        return 0;    /**< @return always 0 for QByteArray */
    }
    qint64 lastByte() const override {
        return bytesWritten();    /**< @return the number of bytes written so far */
    }

    QByteArray& m_Array;
};

#endif
