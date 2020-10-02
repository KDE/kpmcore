/*
    SPDX-FileCopyrightText: 2018 Caio Jordão Carvalho <caiojcarvalho@gmail.com>
    SPDX-FileCopyrightText: 2020 Arnaud Ferraris <arnaud.ferraris@collabora.com>
    SPDX-FileCopyrightText: 2020 Gaël PORTAY <gael.portay@collabora.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef LINUXRAIDMEMBER_H
#define LINUXRAIDMEMBER_H

#include "util/libpartitionmanagerexport.h"

#include "fs/filesystem.h"

class Report;

class QString;

namespace FS
{
/** A linux_raid_member file system.
    @author Caio Jordão Carvalho <caiojcarvalho@gmail.com>
 */
class LIBKPMCORE_EXPORT linuxraidmember : public FileSystem
{
public:
    linuxraidmember(qint64 firstsector, qint64 lastsector, qint64 sectorsused, const QString& label, const QVariantMap& features = {});
};
}

#endif // LINUXRAIDMEMBER_H
