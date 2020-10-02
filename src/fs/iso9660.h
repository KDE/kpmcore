/*
    SPDX-FileCopyrightText: 2017 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2020 Arnaud Ferraris <arnaud.ferraris@collabora.com>
    SPDX-FileCopyrightText: 2020 Gaël PORTAY <gael.portay@collabora.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KPMCORE_ISO9660_H
#define KPMCORE_ISO9660_H

#include "util/libpartitionmanagerexport.h"

#include "fs/filesystem.h"

class Report;

class QString;

namespace FS
{
/** A iso9660 file system.
    @author Andrius Štikonas <andrius@stikonas.eu>
 */
class LIBKPMCORE_EXPORT iso9660 : public FileSystem
{
public:
    iso9660(qint64 firstsector, qint64 lastsector, qint64 sectorsused, const QString& label, const QVariantMap& features = {});

};

}
#endif
