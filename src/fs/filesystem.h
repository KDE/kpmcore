/*************************************************************************
 *  Copyright (C) 2012 by Volker Lanz <vl@fidra.de>                      *
 *  Copyright (C) 2015 by Teo Mrnjavac <teo@kde.org>                     *
 *  Copyright (C) 2016-2018 by Andrius Štikonas <andrius@stikonas.eu>    *
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

#ifndef KPMCORE_FILESYSTEM_H
#define KPMCORE_FILESYSTEM_H

#include "util/libpartitionmanagerexport.h"

#include <QList>
#include <QStringList>
#include <QString>
#include <QtGlobal>
#include <QUrl>

#include <memory>
#include <vector>

class QColor;
class QValidator;
class Device;
class Report;
struct FileSystemPrivate;

/** Base class for all FileSystems.

    Represents a file system and handles support for various types of operations that can
    be performed on those.

    @author Volker Lanz <vl@fidra.de>
 */
class LIBKPMCORE_EXPORT FileSystem
{
    Q_DISABLE_COPY(FileSystem)

public:
    class SupportTool
    {
    public:
        explicit SupportTool(const QString& n = QString(), const QUrl& u = QUrl()) : name(n), url(u) {}

        const QString name;
        const QUrl url;
    };

    /** Supported FileSystem types */
    enum Type : int {
        Unknown,
        Extended,

        Ext2,
        Ext3,
        Ext4,
        LinuxSwap,
        Fat16,
        Fat32,
        Ntfs,
        ReiserFS,
        Reiser4,
        Xfs,
        Jfs,
        Hfs,
        HfsPlus,
        Ufs,
        Unformatted,
        Btrfs,
        Hpfs,
        Luks,
        Ocfs2,
        Zfs,
        Exfat,
        Nilfs2,
        Lvm2_PV,
        F2fs,
        Udf,
        Iso9660,
        Luks2,
        Fat12,
        LinuxRaidMember,

        __lastType
    };

    /** The type of support for a given FileSystem action */
    enum CommandSupportType {
        cmdSupportNone = 0,             /**< no support */
        cmdSupportCore = 1,             /**< internal support */
        cmdSupportFileSystem = 2,       /**< supported by some external command */
        cmdSupportBackend = 4           /**< supported by the backend */
    };

    static const std::vector<QColor> defaultColorCode;

    Q_DECLARE_FLAGS(CommandSupportTypes, CommandSupportType)

protected:
    FileSystem(qint64 firstsector, qint64 lastsector, qint64 sectorsused, const QString& label, FileSystem::Type type);

public:
    virtual ~FileSystem();

public:
    virtual void init() {}
    virtual void scan(const QString& deviceNode);
    virtual qint64 readUsedCapacity(const QString& deviceNode) const;
    virtual QString readLabel(const QString& deviceNode) const;
    virtual bool create(Report& report, const QString& deviceNode);
    virtual bool createWithLabel(Report& report, const QString& deviceNode, const QString& label);
    virtual bool resize(Report& report, const QString& deviceNode, qint64 newLength) const;
    virtual bool resizeOnline(Report& report, const QString& deviceNode, const QString& mountPoint, qint64 newLength) const;
    virtual bool move(Report& report, const QString& deviceNode, qint64 newStartSector) const;
    virtual bool writeLabel(Report& report, const QString& deviceNode, const QString& newLabel);
    virtual bool writeLabelOnline(Report& report, const QString& deviceNode, const QString& mountPoint, const QString& newLabel);
    virtual bool copy(Report& report, const QString& targetDeviceNode, const QString& sourceDeviceNode) const;
    virtual bool backup(Report& report, const Device& sourceDevice, const QString& deviceNode, const QString& filename) const;
    virtual bool remove(Report& report, const QString& deviceNode) const;
    virtual bool check(Report& report, const QString& deviceNode) const;
    virtual bool updateUUID(Report& report, const QString& deviceNode) const;
    virtual QString readUUID(const QString& deviceNode) const;
    virtual bool updateBootSector(Report& report, const QString& deviceNode) const;

    virtual CommandSupportType supportGetUsed() const {
        return cmdSupportNone;    /**< @return CommandSupportType for getting used capacity */
    }
    virtual CommandSupportType supportGetLabel() const {
        return cmdSupportNone;    /**< @return CommandSupportType for reading label*/
    }
    virtual CommandSupportType supportCreate() const {
        return cmdSupportNone;    /**< @return CommandSupportType for creating */
    }
    virtual CommandSupportType supportCreateWithLabel() const {
        return cmdSupportNone;    /**< @return CommandSupportType for creating */
    }
    virtual CommandSupportType supportGrow() const {
        return cmdSupportNone;    /**< @return CommandSupportType for growing */
    }
    virtual CommandSupportType supportGrowOnline() const {
        return cmdSupportNone;    /**< @return CommandSupportType for online growing */
    }
    virtual CommandSupportType supportShrink() const {
        return cmdSupportNone;    /**< @return CommandSupportType for shrinking */
    }
    virtual CommandSupportType supportShrinkOnline() const {
        return cmdSupportNone;    /**< @return CommandSupportType for shrinking */
    }
    virtual CommandSupportType supportMove() const {
        return cmdSupportNone;    /**< @return CommandSupportType for moving */
    }
    virtual CommandSupportType supportCheck() const {
        return cmdSupportNone;    /**< @return CommandSupportType for checking */
    }
    virtual CommandSupportType supportCheckOnline() const {
        return cmdSupportNone;    /**< @return CommandSupportType for checking */
    }
    virtual CommandSupportType supportCopy() const {
        return cmdSupportNone;    /**< @return CommandSupportType for copying */
    }
    virtual CommandSupportType supportBackup() const {
        return cmdSupportNone;    /**< @return CommandSupportType for backing up */
    }
    virtual CommandSupportType supportSetLabel() const {
        return cmdSupportNone;    /**< @return CommandSupportType for setting label */
    }
    virtual CommandSupportType supportSetLabelOnline() const {
        return cmdSupportNone;    /**< @return CommandSupportType for setting label of mounted file systems */
    }
    virtual CommandSupportType supportUpdateUUID() const {
        return cmdSupportNone;    /**< @return CommandSupportType for updating the UUID */
    }
    virtual CommandSupportType supportGetUUID() const {
        return cmdSupportNone;    /**< @return CommandSupportType for reading the UUID */
    }

    virtual qint64 minCapacity() const;
    virtual qint64 maxCapacity() const;
    virtual int maxLabelLength() const;
    virtual QValidator* labelValidator(QObject *parent = nullptr) const;

    virtual SupportTool supportToolName() const;
    virtual bool supportToolFound() const;

    /**
     * Returns the (possibly translated) name of the type of this filesystem.
     * @see nameForType()
     */
    virtual QString name(const QStringList& languages = {}) const;

    /**
     * @return the FileSystem's type
     */
    virtual FileSystem::Type type() const;

    /**
     * Returns the name of the given filesystem type. If @p languages
     * is an empty list, uses the translated name of the filesystem,
     * in the default locale. If languages is {"C"}, an untranslated
     * string is returned. Passing other lists of language identifiers
     * may yield unpredicatable results -- see the documentation of
     * KLocalizedString() for details on the way toString() is used.
     * Returns a single QString with the name.
     */
    static QString nameForType(FileSystem::Type t, const QStringList& languages = {});
    static QList<FileSystem::Type> types();
    static FileSystem::Type typeForName(const QString& s, const QStringList& languages = {});
    static FileSystem::Type detectFileSystem(const QString& partitionPath);
    static QString detectMountPoint(FileSystem* fs, const QString& partitionPath);
    static bool detectMountStatus(FileSystem* fs, const QString& partitionPath);

    /**< @return true if this FileSystem can be mounted */
    virtual bool canMount(const QString& deviceNode, const QString& mountPoint) const;
    virtual bool canUnmount(const QString&) const {
        return true;    /**< @return true if this FileSystem can be unmounted */
    }

    virtual QString mountTitle() const;
    virtual QString unmountTitle() const;

    virtual bool mount(Report& report, const QString& deviceNode, const QString& mountPoint);
    virtual bool unmount(Report& report, const QString& deviceNode);

    /**< @return the FileSystem's first sector */
    qint64 firstSector() const;

    /**< @return the FileSystem's last sector */
    qint64 lastSector() const;

    qint64 length() const {
        return lastSector() - firstSector() + 1;    /**< @return the FileSystem's length */
    }
    qint64 firstByte() const {
        return firstSector() * sectorSize();    /**< @return the FileSystem's first byte */
    }
    qint64 lastByte() const {
        return firstByte() + length() * sectorSize() - 1;    /**< @return the FileSystem's last byte */
    }

    /**< @param s the new first sector */
    void setFirstSector(qint64 s);

    /**< @param s the new last sector */
    void setLastSector(qint64 s);

    void move(qint64 newStartSector);

    /**< @return the FileSystem's label */
    const QString& label() const;

    /**< @return the sector size in the underlying Device */
    qint64 sectorSize() const;

    /**< @return the sectors in use on the FileSystem */
    qint64 sectorsUsed() const;

    /**< @return the FileSystem's UUID */
    const QString& uuid() const;

    /**< @param s the new value for sector size */
    void setSectorSize(qint64 s);

    /**< @param s the new value for sectors in use */
    void setSectorsUsed(qint64 s);

    /**< @param s the new label */
    void setLabel(const QString& s);

    /**< @param s the new UUID */
    void setUUID(const QString& s);

protected:
    static bool findExternal(const QString& cmdName, const QStringList& args = QStringList(), int exptectedCode = 1);

    std::unique_ptr<FileSystemPrivate> d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(FileSystem::CommandSupportTypes)

#endif
