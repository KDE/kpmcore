/*************************************************************************
 *  Copyright (C) 2017 by Andrius Štikonas <andrius@stikonas.eu>         *
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

#include <QList>
#include <QString>

enum FstabEntryType { deviceNode, uuid, label, partlabel, partuuid, comment };

/** Base class for fstab handling.

    FstabEntry stores a single line of /etc/fstab file which can be a comment

    @author Andrius Štikonas <andrius@stikonas.eu>
*/

class LIBKPMCORE_EXPORT FstabEntry
{
public:
    FstabEntry(const QString& fsSpec, const QString& mountPoint, const QString& type, const QString& options, int dumpFreq = 0, int passNumber = 0, const QString& comment = QString());

    const QString& fsSpec() const {
        return m_fsSpec; /**< @return the fs_spec field of fstab entry */
    }
    const QString& deviceNode() const {
        return m_deviceNode; /**< @return the device node corresponding to fs_spec entry */
    }
    const QString& mountPoint() const {
        return m_mountPoint; /**< @return the mount point (target) for the file system */
    }
    const QString& type() const {
        return m_type; /**< @return the type of the file system */
    }
    const QStringList& options() const {
        return m_options; /**< @return the mount options associated with the file system */
    }
    int dumpFreq() const {
        return m_dumpFreq; /**< @return the fs_freq field of fstab entry */
    }
    int passNumber() const {
        return m_passNumber; /**< @return the fs_passno field of fstab entry */
    }
    const QString& comment() const {
        return m_comment; /**< @return commented part of the line in fstab file */
    }
    FstabEntryType entryType() const {
        return m_entryType; /**< @return the type of fstab entry, e.g. device node or UUID or comment only */
    }

    void setFsSpec(const QString& s);
    void setMountPoint(const QString& s) {
        m_mountPoint = s; /**< @param s the new value for the mount point */
    }
    void setOptions(const QStringList& s) {
        m_options = s; /**< @param s the new list with the mount options */
    }
    void setDumpFreq(int s) {
        m_dumpFreq = s; /**< @param s the new value for the dump frequency */
    }
    void setPassNumber(int s) {
        m_passNumber = s; /**< @param s the new value for the pass number */
    }
private:
    QString m_fsSpec;
    QString m_deviceNode;
    QString m_mountPoint;
    QString m_type;
    QStringList m_options;
    int m_dumpFreq;
    int m_passNumber;
    QString m_comment;
    FstabEntryType m_entryType;
};

typedef QList<FstabEntry> FstabEntryList;

LIBKPMCORE_EXPORT FstabEntryList readFstabEntries(const QString& fstabPath = QStringLiteral("/etc/fstab"));
LIBKPMCORE_EXPORT QStringList possibleMountPoints(const QString& deviceNode, const QString& fstabPath = QStringLiteral("/etc/fstab"));
LIBKPMCORE_EXPORT bool writeMountpoints(const FstabEntryList fstabEntries, const QString& filename = QStringLiteral("/etc/fstab"));

#endif
