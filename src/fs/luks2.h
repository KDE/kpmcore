/*
    SPDX-FileCopyrightText: 2017 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2020 Arnaud Ferraris <arnaud.ferraris@collabora.com>
    SPDX-FileCopyrightText: 2020 Gaël PORTAY <gael.portay@collabora.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KPMCORE_LUKS2_H
#define KPMCORE_LUKS2_H

#include "util/libpartitionmanagerexport.h"

#include "fs/luks.h"

#include <QtGlobal>

class QString;

namespace FS
{
/** A LUKS2 crypto file system.
    @author Andrius Štikonas <andrius@stikonas.eu>
*/
class LIBKPMCORE_EXPORT luks2 : public luks
{
public:
    luks2(qint64 firstsector, qint64 lastsector, qint64 sectorsused, const QString& label, const QVariantMap& features = {});
    ~luks2() override;

    bool create(Report& report, const QString& deviceNode) override;
    bool resize(Report& report, const QString& deviceNode, qint64 length) const override;

    FileSystem::Type type() const override;

    luks::KeyLocation keyLocation();
};
}

#endif
