/*
    SPDX-FileCopyrightText: 2008-2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2008 Laurent Montel <montel@kde.org>
    SPDX-FileCopyrightText: 2015 Chris Campbell <c.j.campbell@ed.ac.uk>
    SPDX-FileCopyrightText: 2016 Teo Mrnjavac <teo@kde.org>
    SPDX-FileCopyrightText: 2016-2017 Andrius Štikonas <andrius@stikonas.eu>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KPMCORE_PARTITIONROLE_H
#define KPMCORE_PARTITIONROLE_H

#include "util/libpartitionmanagerexport.h"

#include <QtGlobal>
#include <QStringList>

/** A Partition's role.

    Each Partition has a PartitionRole: It can be primary, extended, logical or represent unallocated space on the Device.

    @author Volker Lanz <vl@fidra.de>
*/
class LIBKPMCORE_EXPORT PartitionRole
{
public:
    /** A Partition's role: What kind of Partition is it? */
    enum Role {
        None = 0,           /**< None at all */
        Primary = 1,        /**< Primary */
        Extended = 2,       /**< Extended */
        Logical = 4,        /**< Logical inside an extended */
        Unallocated = 8,    /**< No real Partition, just unallocated space */
        Luks = 16,          /**< Encrypted partition with LUKS key management */
        Lvm_Lv = 32,        /**< Logical Volume of LVM */

        Any = 255           /**< In case we're looking for a Partition with a PartitionRole, any will do */
    };

    Q_DECLARE_FLAGS(Roles, Role)

public:
    explicit PartitionRole(Roles r) : m_Roles(r) {} /**< Creates a new PartitionRole object */
    Roles roles() const {
        return m_Roles;    /**< @return the roles as bitfield */
    }
    bool has(Role r) const {
        return roles() & r;    /**< @param r the role to check @return true if the role is set */
    }

    bool operator==(const PartitionRole& other) const {
        return m_Roles == other.m_Roles;    /**< @param other object to compare with @return true if the same */
    }
    bool operator!=(const PartitionRole& other) const {
        return !operator==(other);    /**< @param other object to compare with @return true if not the same */
    }

    QString toString(const QStringList& languages = {}) const;

private:
    Roles m_Roles;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(PartitionRole::Roles)

#endif
