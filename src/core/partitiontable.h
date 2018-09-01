/*************************************************************************
 *  Copyright (C) 2008, 2010 by Volker Lanz <vl@fidra.de>                *
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

#if !defined(KPMCORE_PARTITIONTABLE_H)

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
    enum TableType : qint8 {
        unknownTableType = -1,

        aix,
        bsd,
        dasd,
        msdos,
        msdos_sectorbased,
        dvh,
        gpt,
        loop,
        mac,
        pc98,
        amiga,
        sun,
        vmd  /* Volume Manager Device */
    };

    /** Partition flags */
    enum Flag : qint32 {
        FlagNone = 0,
        FlagBoot = 1,
        FlagRoot = 2,
        FlagSwap = 4,
        FlagHidden = 8,
        FlagRaid = 16,
        FlagLvm = 32,
        FlagLba = 64,
        FlagHpService = 128,
        FlagPalo = 256,
        FlagPrep = 512,
        FlagMsftReserved = 1024,
        FlagBiosGrub = 2048,
        FlagAppleTvRecovery = 4096,
        FlagDiag = 8192,
        FlagLegacyBoot = 16384,
        FlagMsftData = 32768,
        FlagIrst = 65536,
        FlagEsp [[deprecated]] = FlagBoot
    };

    Q_DECLARE_FLAGS(Flags, Flag)
    Q_FLAG(Flag)

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

    bool isSectorBased(const Device& d) const;

    static const QList<Flag> flagList();
    static QString flagName(Flag f);
    static QStringList flagNames(Flags f);

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

