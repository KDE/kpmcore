/*
    SPDX-FileCopyrightText: 2008-2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2008 Laurent Montel <montel@kde.org>
    SPDX-FileCopyrightText: 2013-2020 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2015 Chris Campbell <c.j.campbell@ed.ac.uk>
    SPDX-FileCopyrightText: 2015 Teo Mrnjavac <teo@kde.org>
    SPDX-FileCopyrightText: 2020 Gaël PORTAY <gael.portay@collabora.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KPMCORE_PARTITION_H
#define KPMCORE_PARTITION_H

#include "core/partitionnode.h"
#include "core/partitionrole.h"
#include "core/partitiontable.h"

#include "util/libpartitionmanagerexport.h"

#include <QtGlobal>
#include <QPointer>

class Device;
class OperationStack;
class CoreBackendPartitionTable;
class PartitionAlignment;

class PartResizerWidget;
class ResizeDialog;
class InsertDialog;
class NewDialog;
class EditMountPointDialog;
class PartPropsDialog;
class SizeDialogBase;

class CreateFileSystemOperation;
class RestoreOperation;
class SetPartFlagsOperation;
class CopyOperation;
class NewOperation;
class ResizeOperation;

class SetPartGeometryJob;
class CreatePartitionJob;
class SetPartFlagsJob;
class RestoreFileSystemJob;

class FileSystem;

class Report;

class QString;
class QTextStream;

/** A partition or some unallocated space on a Device.

    Represent partitions in a PartitionTable on a Device. Partitions can be unallocated, thus not all
    instances really are partitions in the way the user would see them.

    Extended partitions have child objects that represent the logicals inside them.

    @see PartitionTable, Device, FileSystem
    @author Volker Lanz <vl@fidra.de>
*/
class LIBKPMCORE_EXPORT Partition : public PartitionNode
{

public:
    /** A Partition state -- where did it come from? */
    enum State {
        None,      /**< exists on disk */
        New,       /**< from a NewOperation */
        Copy,      /**< from a CopyOperation */
        Restore,   /**< from a RestoreOperation */
    };

    Partition(PartitionNode* parent, const Device& device, const PartitionRole& role, FileSystem* fs, qint64 sectorStart, qint64 sectorEnd, QString partitionPath, PartitionTable::Flags availableFlags = PartitionTable::Flag::None, const QString& mountPoint = QString(), bool mounted = false, PartitionTable::Flags activeFlags = PartitionTable::Flag::None, State state = State::None);
    ~Partition() override;

    Partition(const Partition& other, PartitionNode* parent = nullptr);
    Partition& operator=(const Partition&);

    bool operator==(const Partition& other) const;
    bool operator!=(const Partition& other) const;

    qint32 number() const {
        return m_Number;    /**< @return the Partition's device number, e.g. 7 for /dev/sdd7 */
    }

    bool isRoot() const override {
        return false;    /**< @return always false for Partition */
    }

    const PartitionTable* partitionTable() const;

    PartitionNode* parent() override {
        return m_Parent;    /**< @return the Partition's parent PartitionNode */
    }
    const PartitionNode* parent() const override {
        return m_Parent;    /**< @return the Partition's parent PartitionNode */
    }

    Partitions& children() override {
        return m_Children;    /**< @return the Partition's children. empty for non-extended. */
    }
    const Partitions& children() const override {
        return m_Children;    /**< @return the Partition's children. empty for non-extended. */
    }
    const QString& devicePath() const {
        return m_DevicePath;    /**< @return the Partition's device path, e.g. /dev/sdd */
    }
    const QString& partitionPath() const {
        return m_PartitionPath;    /**< @return the Partition's path, e.g. /dev/sdd1 */
    }
    const QString& label() const {
        return m_Label;    /**< @return the GPT Partition label */
    }
    const QString& type() const {
        return m_Type;    /**< @return the GPT Partition type */
    }
    const QString& uuid() const {
        return m_UUID;    /**< @return the GPT Partition UUID */
    }
    quint64 attributes() const {
        return m_Attributes;    /**< @return the GPT Partition attributes */
    }
    qint64 firstSector() const {
        return m_FirstSector;    /**< @return the Partition's first sector on the Device */
    }
    qint64 lastSector() const {
        return m_LastSector;    /**< @return the Partition's last sector on the Device */
    }
    qint64 firstByte() const {
        return firstSector() * sectorSize();    /**< @return the Partition's first byte on the Device */
    }
    qint64 lastByte() const {
        return firstByte() + length() * sectorSize() - 1;    /**< @return the Partition's last byte on the Device */
    }
    qint64 sectorsUsed() const;
    qint64 sectorSize() const {
        return m_SectorSize;    /**< @return the sector size on the Partition's Device */
    }
    qint64 length() const {
        return lastSector() - firstSector() + 1;    /**< @return the length of the Partition */
    }
    qint64 capacity() const {
        return length() * sectorSize();    /**< @return the capacity of the Partition in bytes */
    }
    qint64 used() const {
        return sectorsUsed() < 0 ? -1 : sectorsUsed() * sectorSize();    /**< @return the number of used sectors in the Partition's FileSystem */
    }
    qint64 available() const {
        return sectorsUsed() < 0 ? -1 : capacity() - used();    /**< @return the number of free sectors in the Partition's FileSystem */
    }
    qint64 minimumSectors() const;
    qint64 maximumSectors() const;
    qint64 maxFirstSector() const;
    qint64 minLastSector() const;

    QString deviceNode() const;

    const PartitionRole& roles() const {
        return m_Roles;    /**< @return the Partition's role(s) */
    }

    const QString& mountPoint() const {
        return m_MountPoint;    /**< @return the Partition's mount point */
    }

    PartitionTable::Flags activeFlags() const {
        return m_ActiveFlags;    /**< @return the flags currently set for this Partition */
    }
    PartitionTable::Flags availableFlags() const {
        return m_AvailableFlags;    /**< @return the flags available for this Partition */
    }
    bool isMounted() const {
        return m_IsMounted;    /**< @return true if Partition is mounted */
    }
    bool isFileSystemNullptr() {
        return m_FileSystem == nullptr;
    }
    FileSystem& fileSystem() {
        return *m_FileSystem;    /**< @return the Partition's FileSystem */
    }
    const FileSystem& fileSystem() const {
        return *m_FileSystem;    /**< @return the Partition's FileSystem */
    }
    State state() const {
        return m_State;    /**< @return the Partition's state */
    }
    bool hasChildren() const;

    bool mount(Report& report);
    bool unmount(Report& report);

    bool canMount() const;
    bool canUnmount() const;

    void adjustLogicalNumbers(qint32 deletedNumber, qint32 insertedNumber) const;
    void checkChildrenMounted();

    void setFirstSector(qint64 s) {
        m_FirstSector = s;
    }
    void setLastSector(qint64 s) {
        m_LastSector = s;
    }

    void setLabel(const QString& s) {
        m_Label = s;    /**< @param s the new label */
    }
    void setType(const QString& s) {
        m_Type = s;    /**< @param s the new type */
    }
    void setUUID(const QString& s) {
        m_UUID = s;    /**< @param s the new UUID */
    }
    void setAttributes(quint64 f) {
        m_Attributes = f;    /**< @param f the new attributes */
    }

    void append(Partition* p) override {
        m_Children.append(p);
        std::sort(m_Children.begin(), m_Children.end(), [] (const Partition *a, const Partition *b) -> bool {return a->firstSector() < b->firstSector();});
    }
    void setDevicePath(const QString& s) {
        m_DevicePath = s;
    }
    void setPartitionPath(const QString& s);
    void setRoles(const PartitionRole& r) {
        m_Roles = r;
    }
    void setMountPoint(const QString& s) {
        m_MountPoint = s;
    }
    void setFlags(PartitionTable::Flags f) {
        m_ActiveFlags = f;
    }
    void setSectorSize(qint32 s) {
        m_SectorSize = s;
    }
    void move(qint64 newStartSector);
    void setMounted(bool b);

    void setFlag(PartitionTable::Flag f) {
        m_ActiveFlags = m_ActiveFlags.setFlag(f);
    }
    void unsetFlag(PartitionTable::Flag f) {
        m_ActiveFlags = m_ActiveFlags.setFlag(f, false);
    }
    void setParent(PartitionNode* p) {
        m_Parent = p;
    }
    void setFileSystem(FileSystem* fs);
    void setState(State s) {
        m_State = s;
    }
    void deleteFileSystem();

private:
    void setNumber(qint32 n) {
        m_Number = n;
    }

    qint32 m_Number = 0;
    Partitions m_Children;
    QPointer< PartitionNode > m_Parent = nullptr;
    FileSystem* m_FileSystem = nullptr;
    PartitionRole m_Roles;
    qint64 m_FirstSector = 0;
    qint64 m_LastSector = 0;
    QString m_DevicePath;
    QString m_Label;
    QString m_Type;
    QString m_UUID;
    quint64 m_Attributes = 0;
    QString m_PartitionPath;
    QString m_MountPoint;
    PartitionTable::Flags m_AvailableFlags;
    PartitionTable::Flags m_ActiveFlags;
    bool m_IsMounted = false;
    qint64 m_SectorSize = 0;
    State m_State = None;
};

QTextStream& operator<<(QTextStream& stream, const Partition& p);

#endif
