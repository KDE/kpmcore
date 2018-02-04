/*************************************************************************
 *  Copyright (C) 2008 by Volker Lanz <vl@fidra.de>                      *
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

#include "core/partition.h"

#include "core/device.h"

#include "fs/filesystem.h"
#include "fs/filesystemfactory.h"
#include "fs/luks.h"

#include "util/externalcommand.h"
#include "util/report.h"

#include <QRegularExpression>
#include <QStorageInfo>
#include <QString>
#include <QStringList>
#include <QTextStream>

#include <KLocalizedString>

/** Creates a new Partition object.
    @param parent the Partition's parent. May be another Partition (for logicals) or a PartitionTable. Must not be nullptr.
    @param device the Device this Partition is on.
    @param role the Partition's role(s)
    @param fs pointer to the Partition's FileSystem object. The Partition object will take ownership of this.
    @param sectorStart the first sector of the Partition on its Device
    @param sectorEnd the last sector of the Partition on its Device
    @param partitionPath the Partition's path, e.g. /dev/sda4 or /dev/mmcblk0p1
    @param availableFlags the flags available for this Partition
    @param mountPoint mount point for this Partition
    @param mounted true if the Partition is mounted
    @param activeFlags active flags for this Partition
    @param state the Partition's state
*/
Partition::Partition(PartitionNode* parent, const Device& device, const PartitionRole& role, FileSystem* fs, qint64 sectorStart, qint64 sectorEnd, QString partitionPath, PartitionTable::Flags availableFlags, const QString& mountPoint, bool mounted, PartitionTable::Flags activeFlags, State state) :
    PartitionNode(),
    m_Children(),
    m_Parent(parent),
    m_FileSystem(fs),
    m_Roles(role),
    m_FirstSector(sectorStart),
    m_LastSector(sectorEnd),
    m_DevicePath(device.deviceNode()),
    m_MountPoint(mountPoint),
    m_AvailableFlags(availableFlags),
    m_ActiveFlags(activeFlags),
    m_IsMounted(mounted),
    m_State(state)
{
    setPartitionPath(partitionPath);
    Q_ASSERT(m_Parent);
    m_SectorSize = device.logicalSize();
}

/** Destroys a Partition, destroying its children and its FileSystem */
Partition::~Partition()
{
    // FIXME: Design flaw: Currently, there are two ways a partition node can get children: Either
    // they're created and inserted as unallocated in PartitionTable (these unallocated then get
    // "converted" to real, new partitions in the GUI) or they're created and appended in the
    // backend plugin. There is however no defined way to remove partitions from parents. This might
    // either cause leaks (a partition is removed from the parent's list of children but never
    // deleted) or, worse, crashes (a partition is deleted but not removed from the parent's
    // list of children). As a workaround, always remove a partition from its parent here in the dtor.
    // This presumably fixes 232092, but backporting is too risky until we're sure this doesn't cause
    // side-effects.
    if (m_Parent)
        parent()->remove(this);
    clearChildren();
    deleteFileSystem();
}

/** @param other Partition to copy
*/
Partition::Partition(const Partition& other, PartitionNode* parent) :
    PartitionNode(),
    m_Children(),
    m_Parent(other.m_Parent),
    m_FileSystem(FileSystemFactory::create(other.fileSystem())),
    m_Roles(other.m_Roles),
    m_FirstSector(other.m_FirstSector),
    m_LastSector(other.m_LastSector),
    m_DevicePath(other.m_DevicePath),
    m_Label(other.m_Label),
    m_UUID(other.m_UUID),
    m_MountPoint(other.m_MountPoint),
    m_AvailableFlags(other.m_AvailableFlags),
    m_ActiveFlags(other.m_ActiveFlags),
    m_IsMounted(other.m_IsMounted),
    m_SectorSize(other.m_SectorSize),
    m_State(other.m_State)
{
    if ( parent )
        m_Parent = parent;

    setPartitionPath(other.m_PartitionPath);
    for (const auto &child : other.children()) {
        Partition* p = new Partition(*child, this);
        m_Children.append(p);
    }
}

/** @param other Partition to assign from */
Partition& Partition::operator=(const Partition& other)
{
    if (&other == this)
        return *this;

    clearChildren();

    for (const auto &child : other.children()) {
        Partition* p = new Partition(*child);
        p->setParent(this);
        m_Children.append(p);
    }

    m_Number = other.m_Number;
    m_FileSystem = FileSystemFactory::create(other.fileSystem());
    m_Roles = other.m_Roles;
    m_FirstSector = other.m_FirstSector;
    m_LastSector = other.m_LastSector;
    m_DevicePath = other.m_DevicePath;
    m_Label = other.m_Label;
    m_UUID = other.m_UUID;
    m_PartitionPath = other.m_PartitionPath;
    m_MountPoint = other.m_MountPoint;
    m_AvailableFlags = other.m_AvailableFlags;
    m_ActiveFlags = other.m_ActiveFlags;
    m_IsMounted = other.m_IsMounted;
    m_SectorSize = other.m_SectorSize;
    m_State = other.m_State;

    return *this;
}

bool Partition::operator==(const Partition& other) const
{
    return other.deviceNode() == deviceNode();
}

bool Partition::operator!=(const Partition& other) const
{
    return !(other == *this);
}

/** @return a short descriptive text or, in case the Partition has StateNone, its device node. */
QString Partition::deviceNode() const
{
    if (roles().has(PartitionRole::None) || roles().has(PartitionRole::Unallocated))
        return xi18nc("@item partition name", "unallocated");

    if (state() == StateNew)
        return xi18nc("@item partition name", "New Partition");

    if (state() == StateRestore)
        return xi18nc("@item partition name", "Restored Partition");

    if (state() == StateCopy)
        return xi18nc("@item partition name", "Copy of %1", partitionPath());

    return partitionPath();
}

/** @return the sectors used in the Partition's FileSystem or, in case of an extended partition, the sum of used sectors of the Partition's children */
qint64 Partition::sectorsUsed() const
{
    // Make sure file system exists. In some cases (due to bugs elsewhere?) file system pointer did not exist, especially for unallocated space.
    if (m_FileSystem == nullptr)
        return -1;

    if (!roles().has(PartitionRole::Extended))
        return fileSystem().sectorsUsed();

    qint64 result = 0;
    for (const auto &p : children())
        if (!p->roles().has(PartitionRole::Unallocated))
            result += p->length();

    return result;
}

/** @return the minimum number of sectors this Partition must be long */
qint64 Partition::minimumSectors() const
{
    if (roles().has(PartitionRole::Luks))
        return ( fileSystem().minCapacity() + (4096 * 512) ) / sectorSize(); // 4096 is the default cryptsetup payload offset
    return fileSystem().minCapacity() / sectorSize();
}

/** @return the maximum number of sectors this Partition may be long */
qint64 Partition::maximumSectors() const
{
    return fileSystem().maxCapacity() / sectorSize();
}

/** Adjusts the numbers of logical Partitions for an extended Partition.

    This is required if a logical Partition is deleted or inserted because logicals must be numberd from
    5 onwards without a gap. So if the user deletes Partition number 7 and there is a number 8, 8 becomes the
    "new" 7. And since this happens somewhere in the middle of a DeleteOperation, we have to adjust to that so the
    next Job still finds the Partition it wants to deal with.

    @param deletedNumber the number of a deleted logical or -1 if none has been deleted
    @param insertedNumber the number of an inserted logical or -1 if none has been inserted
*/
void Partition::adjustLogicalNumbers(qint32 deletedNumber, qint32 insertedNumber) const
{
    if (!roles().has(PartitionRole::Extended))
        return;

    for (const auto &p : children()) {
        QString path = p->partitionPath();
        path.remove(QRegularExpression(QStringLiteral("(\\d+$)")));
        if (deletedNumber > 4 && p->number() > deletedNumber)
            p->setPartitionPath(path + QString::number(p->number() - 1));
        else if (insertedNumber > 4 && p->number() >= insertedNumber)
            p->setPartitionPath(path + QString::number(p->number() + 1));
    }
}

/** @return the highest sector number an extended Partition can begin at */
qint64 Partition::maxFirstSector() const
{
    qint64 rval = -1;

    for (const auto &child : children())
        if (!child->roles().has(PartitionRole::Unallocated) && (child->firstSector() < rval || rval == -1))
            rval = child->firstSector();

    return rval;
}

/** @return the lowest sector number an extended Partition can end at */
qint64 Partition::minLastSector() const
{
    qint64 rval = -1;

    for (const auto &child : children())
        if (!child->roles().has(PartitionRole::Unallocated) && child->lastSector() > rval)
            rval = child->lastSector();

    return rval;
}

/** @return true if the Partition has children */
bool Partition::hasChildren() const
{
    for (const auto &child : children())
        if (!child->roles().has(PartitionRole::Unallocated))
            return true;

    return false;
}

/** Sets an extended Partition to mounted if any of its children are mounted */
void Partition::checkChildrenMounted()
{
    setMounted(isChildMounted());
}

/** @return true if this Partition can be mounted */
bool Partition::canMount() const
{
    // cannot mount if already mounted
    if (isMounted())  {
        return false;
    }

    if (fileSystem().canMount(deviceNode(), mountPoint())) {
        return true;
    }

    return false;
}

/** @return true if this Partition can be unmounted */
bool Partition::canUnmount() const
{
    return !roles().has(PartitionRole::Extended) && isMounted() && fileSystem().canUnmount(deviceNode());
}

void Partition::setMounted(bool b) {
    m_IsMounted = b;
    if (roles().has(PartitionRole::Luks))
        static_cast<FS::luks*>(m_FileSystem)->setMounted(b);
}

/** Tries to mount a Partition.
    @return true on success
*/
bool Partition::mount(Report& report)
{
    if (isMounted())
        return false;

    bool success = false;

    if (fileSystem().canMount(deviceNode(), mountPoint())) {
        success = fileSystem().mount(report, deviceNode(), mountPoint());
    }

    setMounted(success);

    return success;
}

/** Tries to unmount a Partition.
    @return true on success
*/
bool Partition::unmount(Report& report)
{
    if (!isMounted())
        return false;

    bool success = false;

    if (fileSystem().canUnmount(deviceNode())) {
        success = fileSystem().unmount(report, deviceNode());
    }

    const QString canonicalDeviceNode = QFileInfo(deviceNode()).canonicalFilePath();
    const QList<QStorageInfo> mountedVolumes = QStorageInfo::mountedVolumes();
    for (const QStorageInfo &storage : mountedVolumes) {
        if (QFileInfo(QString::fromLocal8Bit(storage.device())).canonicalFilePath() == canonicalDeviceNode ) {
            success = false;
            break;
        }
    }

    setMounted(!success);

    return success;
}

void Partition::deleteFileSystem()
{
    delete m_FileSystem;
    m_FileSystem = nullptr;
}

void Partition::setPartitionPath(const QString& s)
{
    m_PartitionPath = s;
    QRegularExpression re(QStringLiteral("(\\d+$)"));
    QRegularExpressionMatch rePartitionNumber = re.match(partitionPath());
    if (rePartitionNumber.hasMatch()) {
        setNumber(rePartitionNumber.captured().toInt());
        return;
    }
    setNumber(-1);
}

void Partition::setFileSystem(FileSystem* fs)
{
    m_FileSystem = fs;
}

void Partition::move(qint64 newStartSector)
{
    const qint64 savedLength = length();
    setFirstSector(newStartSector);
    setLastSector(newStartSector + savedLength - 1);
}

QTextStream& operator<<(QTextStream& stream, const Partition& p)
{
    QStringList flagList;

    for (const auto &f : PartitionTable::flagList()) {
        if (p.activeFlags() & f)
            flagList.append(PartitionTable::flagName(f));
    }

    const QString sep(QStringLiteral(";"));

    // number - start - end - type - roles - label - flags
    stream << p.number() << sep
           << p.firstSector() << sep
           << p.lastSector() << sep
           << p.fileSystem().name({ QStringLiteral("C") }) << sep
           << p.roles().toString({ QStringLiteral("C") }) << sep
           << "\"" << p.fileSystem().label() << QStringLiteral("\"") << sep
           << "\"" << flagList.join(QStringLiteral(",")) << QStringLiteral("\"")
           << "\n";

    return stream;
}

