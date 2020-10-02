/*
    SPDX-FileCopyrightText: 2019 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2020 Arnaud Ferraris <arnaud.ferraris@collabora.com>
    SPDX-FileCopyrightText: 2020 Gaël PORTAY <gael.portay@collabora.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KPMCORE_BITLOCKER_H
#define KPMCORE_BITLOCKER_H

#include "util/libpartitionmanagerexport.h"

#include "fs/filesystem.h"

#include <QtGlobal>

class QString;

namespace FS
{
/** A Bitlocker encrypted file system.
    @author Andrius Štikonas <stikonas@kde.org>
 */
class LIBKPMCORE_EXPORT bitlocker : public FileSystem
{
public:
    bitlocker(qint64 firstsector, qint64 lastsector, qint64 sectorsused, const QString& label, const QVariantMap& features = {});

public:
    CommandSupportType supportMove() const override {
        return m_Move;
    }
    CommandSupportType supportCopy() const override {
        return m_Copy;
    }
    CommandSupportType supportBackup() const override {
        return m_Backup;
    }

    bool supportToolFound() const override {
        return true;
    }

public:
    static CommandSupportType m_Move;
    static CommandSupportType m_Copy;
    static CommandSupportType m_Backup;
};
}

#endif
