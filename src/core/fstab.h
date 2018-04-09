/*************************************************************************
 *  Copyright (C) 2017-2018 by Andrius Štikonas <andrius@stikonas.eu>    *
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

#ifndef KPMCORE_FSTAB_H
#define KPMCORE_FSTAB_H

#include "util/libpartitionmanagerexport.h"

#include <memory>

#include <QList>
#include <QString>

struct FstabEntryPrivate;

/** Base class for fstab handling.

    FstabEntry stores a single line of /etc/fstab file which can be a comment

    @author Andrius Štikonas <andrius@stikonas.eu>
*/

class LIBKPMCORE_EXPORT FstabEntry
{
public:
    enum class Type { deviceNode, uuid, label, partlabel, partuuid, comment };

    FstabEntry(const QString& fsSpec, const QString& mountPoint, const QString& type, const QString& options, int dumpFreq = 0, int passNumber = 0, const QString& comment = QString());

    /**
      * @return the fs_spec field of fstab entry
      */
    const QString& fsSpec() const;

    /**
      * @return the device node corresponding to fs_spec entry
      */
    const QString& deviceNode() const;

    /**
      * @return the mount point (target) for the file system
      */
    const QString& mountPoint() const;

    /**
      * @return the type of the file system
      */
    const QString& type() const;

    /**
      * @return the mount options associated with the file system
      */
    const QStringList& options() const;

    /**
      * @return the fs_freq field of fstab entry
      */
    int dumpFreq() const;

    /**
      * @return the fs_passno field of fstab entry
      */
    int passNumber() const;

    /**
      * @return commented part of the line in fstab file
      */
    const QString& comment() const;

    /**
      * @return the type of fstab entry, e.g. device node or UUID or comment only
      */
    Type entryType() const;

    /**
      * @param s the new value for the fs_spec field of fstab entry
      */
    void setFsSpec(const QString& s);

    /**
      * @param s the new value for the mount point
      */
    void setMountPoint(const QString& s);

    /**
      * @param s the new list with the mount options
      */
    void setOptions(const QStringList& s);

    /**
      * @param s the new value for the dump frequency
      */
    void setDumpFreq(int s);

    /**
      * @param s the new value for the pass number
      */
    void setPassNumber(int s);

private:
    std::shared_ptr<FstabEntryPrivate> d;
};

typedef QList<FstabEntry> FstabEntryList;

LIBKPMCORE_EXPORT FstabEntryList readFstabEntries(const QString& fstabPath = QStringLiteral("/etc/fstab"));
LIBKPMCORE_EXPORT QStringList possibleMountPoints(const QString& deviceNode, const QString& fstabPath = QStringLiteral("/etc/fstab"));
LIBKPMCORE_EXPORT bool writeMountpoints(const FstabEntryList& fstabEntries, const QString& filename = QStringLiteral("/etc/fstab"));

#endif
