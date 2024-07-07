/*
    SPDX-FileCopyrightText: 2008-2012 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2008 Laurent Montel <montel@kde.org>
    SPDX-FileCopyrightText: 2015 Chris Campbell <c.j.campbell@ed.ac.uk>
    SPDX-FileCopyrightText: 2016-2019 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2015-2016 Teo Mrnjavac <teo@kde.org>
    SPDX-FileCopyrightText: 2016 Chantara Tith <tith.chantara@gmail.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KPMCORE_PARTITIONTABLE_H
#define KPMCORE_PARTITIONTABLE_H

#include "util/libpartitionmanagerexport.h"

#include "core/partitionnode.h"
#include "core/partitionrole.h"

#include <QList>
#include <QtGlobal>

class Device;
class Partition;
class CoreBackend;

class QTextStream;

/** The partition table (a.k.a Disk Label)

    PartitionTable represents a partition table (or disk label).

    PartitionTable has child nodes that represent Partitions.

    @author Volker Lanz <vl@fidra.de>
*/
class LIBKPMCORE_EXPORT PartitionTable : public PartitionNode
{
    PartitionTable &operator=(const PartitionTable &) = delete;

    friend class CoreBackend;
    friend LIBKPMCORE_EXPORT QTextStream& operator<<(QTextStream& stream, const PartitionTable& ptable);

public:
    enum TableType : int8_t {
        unknownTableType = -1,

        aix,
        bsd,
        dasd,
        msdos,
        msdos_sectorbased [[deprecated]],
        dvh,
        gpt,
        loop,
        mac,
        pc98,
        amiga,
        sun,
        vmd,  /* Volume Manager Device */
        none, /* Single FileSystem devices */
    };

    /** Partition flags */
    enum Flag : uint32_t {
        None = 0x0,
        Boot = 0x1,
        Root = 0x2,
        Swap = 0x4,
        Hidden = 0x8,
        Raid = 0x10,
        Lvm = 0x20,
        Lba = 0x40,
        HpService = 0x80,
        Palo = 0x100,
        Prep = 0x200,
        MsftReserved = 0x400,
        BiosGrub = 0x800,
        AppleTvRecovery = 0x1000,
        Diag = 0x2000,
        LegacyBoot = 0x4000,
        MsftData = 0x8000,
        Irst = 0x100000,
    };

    Q_DECLARE_FLAGS(Flags, Flag)

public:
    PartitionTable(TableType type, qint64 firstUsable, qint64 lastUsable);
    PartitionTable(const PartitionTable& other);
    ~PartitionTable() override;

public:
    PartitionNode* parent() override {
        return nullptr;    /**< @return always nullptr for PartitionTable */
    }
    const PartitionNode* parent() const override {
        return nullptr;    /**< @return always nullptr for PartitionTable */
    }

    bool isRoot() const override {
        return true;    /**< @return always true for PartitionTable */
    }
    bool isReadOnly() const {
        return tableTypeIsReadOnly(type());    /**< @return true if the PartitionTable is read only */
    }

    Partitions& children() override {
        return m_Children;    /**< @return the children in this PartitionTable */
    }
    const Partitions& children() const override {
        return m_Children;    /**< @return the children in this PartitionTable */
    }

    void setType(const Device& d, TableType t);

    void append(Partition* partition) override;

    qint64 freeSectorsBefore(const Partition& p) const;
    qint64 freeSectorsAfter(const Partition& p) const;
    qint64 freeSectors() const;

    bool hasExtended() const;
    Partition* extended() const;

    PartitionRole::Roles childRoles(const Partition& p) const;

    qint32 numPrimaries() const;
    qint32 maxPrimaries() const {
        return m_MaxPrimaries;    /**< @return max number of primary partitions this PartitionTable can handle */
    }

    PartitionTable::TableType type() const {
        return m_Type;    /**< @return the PartitionTable's type */
    }
    const QString typeName() const {
        return tableTypeToName(type());    /**< @return the name of this PartitionTable type */
    }

    qint64 firstUsable() const {
        return m_FirstUsable;
    }
    qint64 lastUsable() const {
        return m_LastUsable;
    }

    void setFirstUsableSector(qint64 s) {
        m_FirstUsable = s;
    }
    void setLastUsableSector(qint64 s) {
        m_LastUsable = s;
    }

    void updateUnallocated(const Device& d);
    void insertUnallocated(const Device& d, PartitionNode* p, qint64 start);

    static const QList<Flag> flagList();
    static QString flagName(Flag f);
    static QStringList flagNames(Flags f);
    static PartitionTable::Flags flagsFromList(const QStringList list);

    static bool getUnallocatedRange(const Device& device, PartitionNode& parent, qint64& start, qint64& end);

    static void removeUnallocated(PartitionNode* p);
    void removeUnallocated();

    static qint64 defaultFirstUsable(const Device& d, TableType t);
    static qint64 defaultLastUsable(const Device& d, TableType t);

    static PartitionTable::TableType nameToTableType(const QString& n);
    static QString tableTypeToName(TableType l);
    static qint32 maxPrimariesForTableType(TableType l);
    static bool tableTypeSupportsExtended(TableType l);
    static bool tableTypeIsReadOnly(TableType l);

protected:
    void setMaxPrimaries(qint32 n) {
        m_MaxPrimaries = n;
    }

private:
    Partitions m_Children;
    qint32 m_MaxPrimaries;
    TableType m_Type;
    qint64 m_FirstUsable;
    qint64 m_LastUsable;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(PartitionTable::Flags)

QTextStream& operator<<(QTextStream& stream, const PartitionTable& ptable);

#endif

