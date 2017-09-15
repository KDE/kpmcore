/*************************************************************************
 *  Copyright (C) 2008 by Volker Lanz <vl@fidra.de>                      *
 *  Copyright (C) 2016 by Teo Mrnjavac <teo@kde.org>                     *
 *                                                                       *
 *  This program is free software; you can redistribute it and/or        *
 *  modify it under the terms of the GNU General Public License as       *
 *  published by the Free Software Foundation; either version 3 of       *
 *  the License, or (at your option) any later version.                  *
 *                                                                       *
 *  This program is distributed in the hope that it will be useful,      *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of       *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        *
 *  GNU General Public License for more details.                         *
 *                                                                       *
 *  You should have received a copy of the GNU General Public License    *
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.*
 *************************************************************************/

#if !defined(KPMCORE_PARTITIONROLE_H)

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
