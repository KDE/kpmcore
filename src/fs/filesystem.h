/*************************************************************************
 *  Copyright (C) 2012 by Volker Lanz <vl@fidra.de>                      *
 *  Copyright (C) 2015 by Teo Mrnjavac <teo@kde.org>                     *
 *  Copyright (C) 2016 by Andrius Štikonas <andrius@stikonas.eu>         *
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

#if !defined(KPMCORE_FILESYSTEM_H)

#define KPMCORE_FILESYSTEM_H
#include "util/libpartitionmanagerexport.h"

#include <QColor>
#include <QList>
#include <QStringList>
#include <QString>
#include <QtGlobal>
#include <QUrl>

#include <array>

class QValidator;
class Device;
class Report;

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
    enum Type {
        Unknown = 0,
        Extended = 1,

        Ext2 = 2,
        Ext3 = 3,
        Ext4 = 4,
        LinuxSwap = 5,
        Fat16 = 6,
        Fat32 = 7,
        Ntfs = 8,
        ReiserFS = 9,
        Reiser4 = 10,
        Xfs = 11,
        Jfs = 12,
        Hfs = 13,
        HfsPlus = 14,
        Ufs = 15,
        Unformatted = 16,
        Btrfs = 17,
        Hpfs = 18,
        Luks = 19,
        Ocfs2 = 20,
        Zfs = 21,
        Exfat = 22,
        Nilfs2 = 23,
        Lvm2_PV = 24,
        F2fs = 25,
        Udf = 26,
        Iso9660 = 27,
        Luks2 = 28,
        Fat12 = 29,

        __lastType = 30
    };

    /** The type of support for a given FileSystem action */
    enum CommandSupportType {
        cmdSupportNone = 0,             /**< no support */
        cmdSupportCore = 1,             /**< internal support */
        cmdSupportFileSystem = 2,       /**< supported by some external command */
        cmdSupportBackend = 4           /**< supported by the backend */
    };

    static const std::array< QColor, __lastType > defaultColorCode;

    Q_DECLARE_FLAGS(CommandSupportTypes, CommandSupportType)

protected:
    FileSystem(qint64 firstsector, qint64 lastsector, qint64 sectorsused, const QString& label, FileSystem::Type t);

public:
    virtual ~FileSystem() {}

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
    virtual FileSystem::Type type() const {
        return m_Type;    /**< @return the FileSystem's type */
    }

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

    qint64 firstSector() const {
        return m_FirstSector;    /**< @return the FileSystem's first sector */
    }
    qint64 lastSector() const {
        return m_LastSector;    /**< @return the FileSystem's last sector */
    }
    qint64 length() const {
        return lastSector() - firstSector() + 1;    /**< @return the FileSystem's length */
    }
    qint64 firstByte() const {
        return firstSector() * sectorSize();    /**< @return the FileSystem's first byte */
    }
    qint64 lastByte() const {
        return firstByte() + length() * sectorSize() - 1;    /**< @return the FileSystem's last byte */
    }

    void setFirstSector(qint64 s) {
        m_FirstSector = s;    /**< @param s the new first sector */
    }
    void setLastSector(qint64 s) {
        m_LastSector = s;    /**< @param s the new last sector */
    }

    void move(qint64 newStartSector);

    const QString& label() const {
        return m_Label;    /**< @return the FileSystem's label */
    }
    qint64 sectorSize() const {
        return m_SectorSize;    /**< @return the sector size in the underlying Device */
    }
    qint64 sectorsUsed() const {
        return m_SectorsUsed;    /**< @return the sectors in use on the FileSystem */
    }
    const QString& uuid() const {
        return m_UUID;    /**< @return the FileSystem's UUID */
    }

    void setSectorSize(qint64 s) {
        m_SectorSize = s;    /**< @param s the new value for sector size */
    }
    void setSectorsUsed(qint64 s) {
        m_SectorsUsed = s;    /**< @param s the new value for sectors in use */
    }
    void setLabel(const QString& s) {
        m_Label = s;    /**< @param s the new label */
    }
    void setUUID(const QString& s) {
        m_UUID = s;    /**< @param s the new UUID */
    }

protected:
    static bool findExternal(const QString& cmdName, const QStringList& args = QStringList(), int exptectedCode = 1);

protected:
    FileSystem::Type m_Type;
    qint64 m_FirstSector;
    qint64 m_LastSector;
    qint64 m_SectorSize;
    qint64 m_SectorsUsed;
    QString m_Label;
    QString m_UUID;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(FileSystem::CommandSupportTypes)

#endif
