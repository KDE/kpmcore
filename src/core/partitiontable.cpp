/*
    SPDX-FileCopyrightText: 2008-2012 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2009 Andrew Coles <andrew.i.coles@googlemail.com>
    SPDX-FileCopyrightText: 2013-2020 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2015-2016 Teo Mrnjavac <teo@kde.org>
    SPDX-FileCopyrightText: 2016 Chantara Tith <tith.chantara@gmail.com>
    SPDX-FileCopyrightText: 2018 Caio Jordão Carvalho <caiojcarvalho@gmail.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

/** @file
*/

#include "core/partitiontable.h"
#include "core/partition.h"
#include "core/device.h"
#include "core/diskdevice.h"
#include "core/lvmdevice.h"
#include "core/partitionalignment.h"
#include "fs/filesystem.h"
#include "fs/filesystemfactory.h"

#include "util/globallog.h"

#include <utility>

#include <KLocalizedString>

#include <QDebug>
#include <QTextStream>

/** Creates a new PartitionTable object with type MSDOS
    @param type name of the PartitionTable type (e.g. "msdos" or "gpt")
*/
PartitionTable::PartitionTable(TableType type, qint64 firstUsable, qint64 lastUsable)
    : PartitionNode()
    , m_Children()
    , m_MaxPrimaries(maxPrimariesForTableType(type))
    , m_Type(type)
    , m_FirstUsable(firstUsable)
    , m_LastUsable(lastUsable)
{
}

/** Copy constructor for PartitionTable.
 * @param other the other PartitionTable.
 */
PartitionTable::PartitionTable(const PartitionTable& other)
    : PartitionNode()
    , m_Children()
    , m_MaxPrimaries(other.m_MaxPrimaries)
    , m_Type(other.m_Type)
    , m_FirstUsable(other.m_FirstUsable)
    , m_LastUsable(other.m_LastUsable)
{
    for (Partitions::const_iterator it = other.m_Children.constBegin();
         it != other.m_Children.constEnd(); ++it)
    {
        m_Children.append(new Partition(**it, this));
    }
}

/** Destroys a PartitionTable object, destroying all children */
PartitionTable::~PartitionTable()
{
    clearChildren();
}

/** Gets the number of free sectors before a given child Partition in this PartitionTable.

    @param p the Partition for which to get the free sectors before
    @returns the number of free sectors before the Partition
*/
qint64 PartitionTable::freeSectorsBefore(const Partition& p) const
{
    const Partition* pred = predecessor(p);

    // due to the space required for extended boot records the
    // below is NOT the same as pred->length()
    if (pred && pred->roles().has(PartitionRole::Unallocated))
        return p.firstSector() - pred->firstSector();

    return 0;
}

/** Gets the number of free sectors after a given child Partition in this PartitionTable.

    @param p the Partition for which to get the free sectors after
    @returns the number of free sectors after the Partition
*/
qint64 PartitionTable::freeSectorsAfter(const Partition& p) const
{
    const Partition* succ = successor(p);

    // due to the space required for extended boot records the
    // below is NOT the same as succ->length()
    if (succ && succ->roles().has(PartitionRole::Unallocated))
        return succ->lastSector() - p.lastSector();

    return 0;
}

qint64 PartitionTable::freeSectors() const
{
    qint64 sectors = 0;
    for (const auto &p : children()) {
        if (p->roles().has(PartitionRole::Unallocated)) {
            sectors += p->length();
        }
    }

    return sectors;
}

/** @return true if the PartitionTable has an extended Partition */
bool PartitionTable::hasExtended() const
{
    for (const auto &p : children())
        if (p->roles().has(PartitionRole::Extended))
            return true;

    return false;
}

/** @return pointer to the PartitionTable's extended Partition or nullptr if none exists */
Partition* PartitionTable::extended() const
{
    for (const auto &p : children())
        if (p->roles().has(PartitionRole::Extended))
            return p;

    return nullptr;
}

/** Gets valid PartitionRoles for a Partition
    @param p the Partition
    @return valid roles for the given Partition
*/
PartitionRole::Roles PartitionTable::childRoles(const Partition& p) const
{
    Q_ASSERT(p.parent());

    PartitionRole::Roles r = p.parent()->isRoot() ? PartitionRole::Primary : PartitionRole::Logical;

    if (r == PartitionRole::Primary && hasExtended() == false && tableTypeSupportsExtended(type()))
        r |= PartitionRole::Extended;

    return r;
}

/** @return the number of primaries in this PartitionTable */
int PartitionTable::numPrimaries() const
{
    int result = 0;

    for (const auto &p : children())
        if (p->roles().has(PartitionRole::Primary) || p->roles().has(PartitionRole::Extended))
            result++;

    return result;
}

/** Appends a Partition to this PartitionTable
    @param partition pointer of the partition to append. Must not be nullptr.
*/
void PartitionTable::append(Partition* partition)
{
    children().append(partition);
    std::sort(children().begin(), children().end(), [] (const Partition *a, const Partition *b) -> bool {return a->firstSector() < b->firstSector();});
}

/** @param f the flag to get the name for
    @returns the flags name or an empty QString if the flag is not known
*/
QString PartitionTable::flagName(Flag f)
{
    switch (f) {
    case PartitionTable::Flag::Boot:
        return xi18nc("@item partition flag", "boot");
    case PartitionTable::Flag::Root:
        return xi18nc("@item partition flag", "root");
    case PartitionTable::Flag::Swap:
        return xi18nc("@item partition flag", "swap");
    case PartitionTable::Flag::Hidden:
        return xi18nc("@item partition flag", "hidden");
    case PartitionTable::Flag::Raid:
        return xi18nc("@item partition flag", "raid");
    case PartitionTable::Flag::Lvm:
        return xi18nc("@item partition flag", "lvm");
    case PartitionTable::Flag::Lba:
        return xi18nc("@item partition flag", "lba");
    case PartitionTable::Flag::HpService:
        return xi18nc("@item partition flag", "hpservice");
    case PartitionTable::Flag::Palo:
        return xi18nc("@item partition flag", "palo");
    case PartitionTable::Flag::Prep:
        return xi18nc("@item partition flag", "prep");
    case PartitionTable::Flag::MsftReserved:
        return xi18nc("@item partition flag", "msft-reserved");
    case PartitionTable::Flag::BiosGrub:
        return xi18nc("@item partition flag", "bios-grub");
    case PartitionTable::Flag::AppleTvRecovery:
        return xi18nc("@item partition flag", "apple-tv-recovery");
    case PartitionTable::Flag::Diag:
        return xi18nc("@item partition flag", "diag");
    case PartitionTable::Flag::LegacyBoot:
        return xi18nc("@item partition flag", "legacy-boot");
    case PartitionTable::Flag::MsftData:
        return xi18nc("@item partition flag", "msft-data");
    case PartitionTable::Flag::Irst:
        return xi18nc("@item partition flag", "irst");
    default:
        break;
    }

    return QString();
}

/** @return list of all flags */
const QList<PartitionTable::Flag> PartitionTable::flagList()
{
    QList<PartitionTable::Flag> rval;

    rval.append(PartitionTable::Flag::Boot);
    rval.append(PartitionTable::Flag::Root);
    rval.append(PartitionTable::Flag::Swap);
    rval.append(PartitionTable::Flag::Hidden);
    rval.append(PartitionTable::Flag::Raid);
    rval.append(PartitionTable::Flag::Lvm);
    rval.append(PartitionTable::Flag::Lba);
    rval.append(PartitionTable::Flag::HpService);
    rval.append(PartitionTable::Flag::Palo);
    rval.append(PartitionTable::Flag::Prep);
    rval.append(PartitionTable::Flag::MsftReserved);
    rval.append(PartitionTable::Flag::BiosGrub);
    rval.append(PartitionTable::Flag::AppleTvRecovery);
    rval.append(PartitionTable::Flag::Diag);
    rval.append(PartitionTable::Flag::LegacyBoot);
    rval.append(PartitionTable::Flag::MsftData);
    rval.append(PartitionTable::Flag::Irst);

    return rval;
}

/** @param flags the flags to get the names for
    @returns QStringList of the flags' names
*/
QStringList PartitionTable::flagNames(Flags flags)
{
    QStringList rval;

    int f = 1;

    QString s;
    while (!(s = flagName(static_cast<PartitionTable::Flag>(f))).isEmpty()) {
        if (flags & f)
            rval.append(s);

        f <<= 1;
    }

    return rval;
}

/** @param list QStringList of the flags' names
    @returns flags corresponding to names
*/
PartitionTable::Flags PartitionTable::flagsFromList(const QStringList list)
{
    Flags flags;

    for (const auto &flag : flagList())
        if (list.contains(flagName(flag)))
            flags.setFlag(flag);

    return flags;
}

bool PartitionTable::getUnallocatedRange(const Device& d, PartitionNode& parent, qint64& start, qint64& end)
{
    if (d.type() == Device::Type::Disk_Device) {
        const DiskDevice& device = dynamic_cast<const DiskDevice&>(d);
        if (!parent.isRoot()) {
            Partition* extended = dynamic_cast<Partition*>(&parent);

            if (extended == nullptr) {
                qWarning() << "extended is null. start: " << start << ", end: " << end << ", device: " << device.deviceNode();
                return false;
            }

            // Leave a track (cylinder aligned) or sector alignment sectors (sector based) free at the
            // start for a new partition's metadata
            start += device.partitionTable()->type() == PartitionTable::msdos ? device.sectorsPerTrack() : PartitionAlignment::sectorAlignment(device);

            // .. and also at the end for the metadata for a partition to follow us, if we're not
            // at the end of the extended partition
            if (end < extended->lastSector())
                end -= device.partitionTable()->type() == PartitionTable::msdos ? device.sectorsPerTrack() : PartitionAlignment::sectorAlignment(device);
        }

        return end - start + 1 >= PartitionAlignment::sectorAlignment(device);
    } else if (d.type() == Device::Type::LVM_Device || d.type() == Device::Type::SoftwareRAID_Device) {
        if (end - start + 1 > 0) {
            return true;
        }
    }
    return false;
}


/** Creates a new unallocated Partition on the given Device.
    @param device the Device to create the new Partition on
    @param parent the parent PartitionNode for the new Partition
    @param start the new Partition's start sector
    @param end the new Partition's end sector
    @return pointer to the newly created Partition object or nullptr if the Partition could not be created
*/
Partition* createUnallocated(const Device& device, PartitionNode& parent, qint64 start, qint64 end)
{
    PartitionRole::Roles r = PartitionRole::Unallocated;

    if (!parent.isRoot())
        r |= PartitionRole::Logical;

    // Mark unallocated space in LVM VG as LVM LV so that pasting can be easily disabled (it does not work yet)
    if (device.type() == Device::Type::LVM_Device)
        r |= PartitionRole::Lvm_Lv;

    if (!PartitionTable::getUnallocatedRange(device, parent, start, end))
        return nullptr;

    return new Partition(&parent, device, PartitionRole(r), FileSystemFactory::create(FileSystem::Type::Unknown, start, end, device.logicalSize()), start, end, QString());
}

/** Removes all unallocated children from a PartitionNode
    @param p pointer to the parent to remove unallocated children from
*/
void PartitionTable::removeUnallocated(PartitionNode* p)
{
    Q_ASSERT(p);

    qint32 i = 0;

    while (i < p->children().size()) {
        Partition* child = p->children()[i];

        if (child->roles().has(PartitionRole::Unallocated)) {
            p->remove(child);
            delete child;
            continue;
        }

        if (child->roles().has(PartitionRole::Extended))
            removeUnallocated(child);

        i++;
    }
}

/**
    @overload
*/
void PartitionTable::removeUnallocated()
{
    removeUnallocated(this);
}

/** Inserts unallocated children for a Device's PartitionTable with the given parent.

    This method inserts unallocated Partitions for a parent, usually the Device this
    PartitionTable is on. It will also insert unallocated Partitions in any extended
    Partitions it finds.

    @warning This method assumes that no unallocated Partitions exist when it is called.

    @param d the Device this PartitionTable and @p p are on
    @param p the parent PartitionNode (may be this or an extended Partition)
    @param start the first sector to begin looking for free space
*/
void PartitionTable::insertUnallocated(const Device& d, PartitionNode* p, qint64 start)
{
    Q_ASSERT(p);

    qint64 lastEnd = start;

    if (d.type() == Device::Type::LVM_Device && !p->children().isEmpty()) {
        // rearranging the sectors of all partitions to keep unallocated space at the end
        lastEnd = 0;
        std::sort(children().begin(), children().end(), [](const Partition* p1, const Partition* p2) { return p1->deviceNode() < p2->deviceNode(); });
        for (const auto &child : children()) {
            qint64 totalSectors = child->length();
            child->setFirstSector(lastEnd);
            child->setLastSector(lastEnd + totalSectors - 1);

            lastEnd += totalSectors;
        }
    } else {
        const auto pChildren = p->children();
        for (const auto &child : pChildren) {
            p->insert(createUnallocated(d, *p, lastEnd, child->firstSector() - 1));

            if (child->roles().has(PartitionRole::Extended))
                insertUnallocated(d, child, child->firstSector());

            lastEnd = child->lastSector() + 1;
        }
    }

    if (d.type() == Device::Type::LVM_Device)
    {
        const LvmDevice& lvm = static_cast<const LvmDevice&>(d);
        p->insert(createUnallocated(d, *p, lastEnd, lastEnd + lvm.freePE() - 1));
    }
    else
    {
        // Take care of the free space between the end of the last child and the end
        // of the device or the extended partition.
        qint64 parentEnd = lastUsable();

        if (!p->isRoot()) {
            Partition* extended = dynamic_cast<Partition*>(p);
            parentEnd = extended ? extended->lastSector() : -1;
            Q_ASSERT(extended);
        }

        if (parentEnd >= firstUsable() && parentEnd >= lastEnd)
            p->insert(createUnallocated(d, *p, lastEnd, parentEnd));
    }
}

/** Updates the unallocated Partitions for this PartitionTable.
    @param d the Device this PartitionTable is on
*/
void PartitionTable::updateUnallocated(const Device& d)
{
    removeUnallocated();
    insertUnallocated(d, this, firstUsable());
}

qint64 PartitionTable::defaultFirstUsable(const Device& d, TableType t)
{
    Q_UNUSED(t)
    if (d.type() == Device::Type::LVM_Device || d.type() == Device::Type::SoftwareRAID_Device || t == PartitionTable::TableType::none) {
        return 0;
    }

    const DiskDevice& diskDevice = dynamic_cast<const DiskDevice&>(d);
    return PartitionAlignment::sectorAlignment(diskDevice);
}

qint64 PartitionTable::defaultLastUsable(const Device& d, TableType t)
{
    if (t == gpt)
        return d.totalLogical() - 1 - 32 - 1;

    return d.totalLogical() - 1;
}

static struct {
    const QLatin1String name; /**< name of partition table type */
    quint32 maxPrimaries; /**< max numbers of primary partitions supported */
    bool canHaveExtended; /**< does partition table type support extended partitions */
    bool isReadOnly; /**< does KDE Partition Manager support this only in read only mode */
    PartitionTable::TableType type; /**< enum type */
} tableTypes[] = {
    { QLatin1String("aix"), 4, false, true, PartitionTable::TableType::aix },
    { QLatin1String("bsd"), 8, false, true, PartitionTable::TableType::bsd },
    { QLatin1String("dasd"), 1, false, true, PartitionTable::TableType::dasd },
    { QLatin1String("msdos"), 4, true, false, PartitionTable::TableType::msdos },
    { QLatin1String("msdos"), 4, true, false, PartitionTable::TableType::msdos_sectorbased },
    { QLatin1String("dos"), 4, true, false, PartitionTable::TableType::msdos_sectorbased },
    { QLatin1String("dvh"), 16, true, true, PartitionTable::TableType::dvh },
    { QLatin1String("gpt"), 128, false, false, PartitionTable::TableType::gpt },
    { QLatin1String("loop"), 1, false, true, PartitionTable::TableType::loop },
    { QLatin1String("mac"), 0xffff, false, true, PartitionTable::TableType::mac },
    { QLatin1String("pc98"), 16, false, true, PartitionTable::TableType::pc98 },
    { QLatin1String("amiga"), 128, false, true, PartitionTable::TableType::amiga },
    { QLatin1String("sun"), 8, false, true, PartitionTable::TableType::sun },
    { QLatin1String("vmd"), 0xffff, false, false, PartitionTable::TableType::vmd },
    { QLatin1String("none"), 1, false, false, PartitionTable::TableType::none },
};

PartitionTable::TableType PartitionTable::nameToTableType(const QString& n)
{
    for (const auto &type : tableTypes)
        if (n == type.name)
            return type.type;

    return PartitionTable::TableType::unknownTableType;
}

QString PartitionTable::tableTypeToName(TableType l)
{
    for (const auto &type : tableTypes)
        if (l == type.type)
            return type.name;

    return xi18nc("@item partition table name", "unknown");
}

qint32 PartitionTable::maxPrimariesForTableType(TableType l)
{
    for (const auto &type : tableTypes)
        if (l == type.type)
            return type.maxPrimaries;

    return 1;
}

bool PartitionTable::tableTypeSupportsExtended(TableType l)
{
    for (const auto &type : tableTypes)
        if (l == type.type)
            return type.canHaveExtended;

    return false;
}

bool PartitionTable::tableTypeIsReadOnly(TableType l)
{
    for (const auto &type : tableTypes)
        if (l == type.type)
            return type.isReadOnly;

    return false;
}

/** Simple heuristic to determine if the PartitionTable is sector aligned (i.e.
    if its Partitions begin at sectors evenly divisable by PartitionAlignment::sectorAlignment().
    @return true if is sector aligned, otherwise false
*/
bool PartitionTable::isSectorBased(const Device& d) const
{
    if (d.type() == Device::Type::Disk_Device) {
        const DiskDevice& diskDevice = dynamic_cast<const DiskDevice&>(d);

        if (type() == PartitionTable::msdos) {
            // the default for empty partition tables is sector based
            if (numPrimaries() == 0)
                return true;

            quint32 numCylinderAligned = 0;
            quint32 numSectorAligned = 0;

            // see if we have more cylinder aligned partitions than sector
            // aligned ones.
            for (const auto &p : children()) {
                if (p->firstSector() % PartitionAlignment::sectorAlignment(diskDevice) == 0)
                    numSectorAligned++;
                else if (p->firstSector() % diskDevice.cylinderSize() == 0)
                    numCylinderAligned++;
            }

            return numSectorAligned >= numCylinderAligned;
        }
        return type() == PartitionTable::msdos_sectorbased;
    }

    return false;
}

void PartitionTable::setType(const Device& d, TableType t)
{
    setFirstUsableSector(defaultFirstUsable(d, t));
    setLastUsableSector(defaultLastUsable(d, t));

    m_Type = t;

    updateUnallocated(d);
}

QTextStream& operator<<(QTextStream& stream, const PartitionTable& ptable)
{
    stream << "type: \"" << ptable.typeName() << "\"\n"
           << "align: \"" << (ptable.type() == PartitionTable::msdos ? "cylinder" : "sector") << "\"\n"
           << "\n# number start end type roles label flags\n";

    QList<const Partition*> partitions;

    for (const auto &p : ptable.children()) {
        if (!p->roles().has(PartitionRole::Unallocated)) {
            partitions.append(p);

            if (p->roles().has(PartitionRole::Extended)) {
                const auto partChildren = p->children();
                for (const auto &child : partChildren) {
                    if (!child->roles().has(PartitionRole::Unallocated))
                        partitions.append(child);
                }
            }
        }
    }

    std::sort(partitions.begin(), partitions.end(), [](const Partition* p1, const Partition* p2) { return p1->number() < p2->number(); });

    for (const auto &p : std::as_const(partitions))
        stream << *p;

    return stream;
}
