/*
    SPDX-FileCopyrightText: 2008 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2014-2017 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2016 Teo Mrnjavac <teo@kde.org>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "core/partitionrole.h"

#include <KLocalizedString>

/** @return the role as string */
QString PartitionRole::toString(const QStringList& languages) const
{
    if (roles() & Unallocated)
        return kxi18nc("@item partition role", "unallocated").toString(languages);

    if (roles() & Logical)
        return kxi18nc("@item partition role", "logical").toString(languages);

    if (roles() & Extended)
        return kxi18nc("@item partition role", "extended").toString(languages);

    if (roles() & Primary)
        return kxi18nc("@item partition role", "primary").toString(languages);

    if (roles() & Luks)
        return kxi18nc("@item partition role", "LUKS").toString(languages);

    if (roles() & Lvm_Lv)
        return kxi18nc("@item partition role", "LVM logical volume").toString(languages);

    return kxi18nc("@item partition role", "none").toString(languages);
}
