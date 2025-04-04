/*
    SPDX-FileCopyrightText: 2008-2011 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2012-2020 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2015-2016 Teo Mrnjavac <teo@kde.org>
    SPDX-FileCopyrightText: 2016 Chantara Tith <tith.chantara@gmail.com>
    SPDX-FileCopyrightText: 2017 Pali Rohár <pali.rohar@gmail.com>
    SPDX-FileCopyrightText: 2018 Caio Jordão Carvalho <caiojcarvalho@gmail.com>
    SPDX-FileCopyrightText: 2019 Yuri Chornoivan <yurchor@ukr.net>
    SPDX-FileCopyrightText: 2020 Arnaud Ferraris <arnaud.ferraris@collabora.com>
    SPDX-FileCopyrightText: 2020 Gaël PORTAY <gael.portay@collabora.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "fs/filesystem.h"
#include "core/fstab.h"

#include "fs/lvm2_pv.h"

#include "backend/corebackend.h"
#include "backend/corebackendmanager.h"

#include "util/externalcommand.h"
#include "util/externalcommand_trustedprefixes.h"
#include "util/capacity.h"
#include "util/helpers.h"

#include <KLocalizedString>

#include <QColor>
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>
#include <QStorageInfo>
#include <QTemporaryDir>

const std::vector<QColor> FileSystem::defaultColorCode =
{
{
    QColor( 220,205,175 ), // unknown
    QColor( 187,249,207 ), // extended
    QColor( 102,121,150 ), // ext2
    QColor( 122,145,180 ), // ext3
    QColor( 143,170,210 ), // ext4
    QColor( 155,155,130 ), // linuxswap
    QColor( 155,155,130 ), // freebsdswap
    QColor( 204,179,215 ), // fat16
    QColor( 229,201,240 ), // fat32
    QColor( 244,214,255 ), // ntfs
    QColor( 216,220,135 ), // reiser
    QColor( 251,255,157 ), // reiser4
    QColor( 200,255,254 ), // xfs
    QColor( 137,200,198 ), // jfs
    QColor( 210,136,142 ), // hfs
    QColor( 240,165,171 ), // hfs+
    QColor( 151,220,134 ), // ufs
    QColor( 220,205,175 ), // unformatted
    QColor( 173,205,255 ), // btrfs
    QColor( 176,155,185 ), // hpfs
    QColor( 170,30,77 ),   // luks
    QColor( 96,140,85 ),   // ocfs2
    QColor( 33,137,108 ),  // zfs
    QColor( 250,230,255 ), // exfat
    QColor( 242,155,104 ), // nilfs2
    QColor( 160,210,180 ), // lvm2 pv
    QColor( 255,170,0 ),   // f2fs
    QColor( 170,120,255 ), // udf
    QColor( 177,82,69 ),   // iso9660
    QColor( 223,39,104 ),  // luks2
    QColor( 204,179,255 ), // fat12
    QColor( 255,100,100 ), // linux_raid_member
    QColor( 110,20,50 ),   // bitlocker
    QColor( 255,155,174 ), // apfs
    QColor( 0,170,255 ),   // minix
    QColor( 100,170,255 ), // bcachefs
}
};

struct FileSystemPrivate {
    FileSystem::Type m_Type;
    qint64 m_FirstSector;
    qint64 m_LastSector;
    qint64 m_SectorSize;
    qint64 m_SectorsUsed;
    QString m_Label;
    QString m_UUID;
    QString m_posixPermissions;
    QStringList m_AvailableFeatures;
    QVariantMap m_Features;
};

/** Creates a new FileSystem object
    @param firstsector the first sector used by this FileSystem on the Device
    @param lastsector the last sector used by this FileSystem on the Device
    @param sectorsused the number of sectors in use on the FileSystem
    @param label the FileSystem label
    @param type the FileSystem type
*/
FileSystem::FileSystem(qint64 firstsector, qint64 lastsector, qint64 sectorsused, const QString& label, FileSystem::Type type) :
    d(std::make_unique<FileSystemPrivate>())
{
    d->m_Type = type;
    d->m_FirstSector = firstsector;
    d->m_LastSector = lastsector;
    d->m_SectorsUsed = sectorsused;
    d->m_Label = label;
    d->m_UUID = QString();
}

/** Creates a new FileSystem object
    @param firstsector the first sector used by this FileSystem on the Device
    @param lastsector the last sector used by this FileSystem on the Device
    @param sectorsused the number of sectors in use on the FileSystem
    @param label the FileSystem label
    @param features the FileSystem features
    @param type the FileSystem type
*/
FileSystem::FileSystem(qint64 firstsector, qint64 lastsector, qint64 sectorsused, const QString& label, const QVariantMap& features, FileSystem::Type type) :
    d(std::make_unique<FileSystemPrivate>())
{
    d->m_Type = type;
    d->m_FirstSector = firstsector;
    d->m_LastSector = lastsector;
    d->m_SectorsUsed = sectorsused;
    d->m_Label = label;
    d->m_Features = features;
    d->m_UUID = QString();
}

FileSystem::~FileSystem()
{
}

QString FileSystem::implPosixPermissions() const
{
    return d->m_posixPermissions;
}

void FileSystem::implSetPosixPermissions(const QString& permissions)
{
    d->m_posixPermissions = permissions;
}


bool FileSystem::execChangePosixPermission(Report& report, const QString& deviceNode)
{
    // do nothing if the posix permissions is not used here.
    if (d->m_posixPermissions.isEmpty()) {
        return true;
    }

    QTemporaryDir tmpDir;

    ExternalCommand mountCmd(report, QStringLiteral("mount"),
                             { deviceNode, tmpDir.path() });

    bool step =  mountCmd.run() && mountCmd.exitCode() == 0;
    if (!step) {
        return false;
    }

    ExternalCommand chmodCmd(report, QStringLiteral("chmod"),
                             // forcing recursive, should be empty but
                             // programming is weird.
                            {
                                d->m_posixPermissions,
                                tmpDir.path(),
                                QStringLiteral("--recursive")
                            });

    const bool chmodStep =  chmodCmd.run() && chmodCmd.exitCode() == 0;

    ExternalCommand umountCmd(report, QStringLiteral("umount"),
                            // forcing recursive, should be empty but
                             // programming is weird.
                            {
                                deviceNode,
                            });

    const bool umountStep =  umountCmd.run() && umountCmd.exitCode() == 0;

    // we can't return false if chmodStep fails because we still need to umount
    // the drive.
    if (!chmodStep) {
        return false;
    }

    if (!umountStep) {
        return false;
    }

    return true;
}

/** Reads the capacity in use on this FileSystem
    @param deviceNode the device node for the Partition the FileSystem is on
    @return the used capacity in bytes or -1 in case of an error
*/
qint64 FileSystem::readUsedCapacity(const QString& deviceNode) const
{
    Q_UNUSED(deviceNode)

    return -1;
}

FileSystem::Type FileSystem::detectFileSystem(const QString& partitionPath)
{
    return CoreBackendManager::self()->backend()->detectFileSystem(partitionPath);
}

QString FileSystem::detectMountPoint(FileSystem* fs, const QString& partitionPath)
{
    if (fs->type() == FileSystem::Type::Lvm2_PV)
        return FS::lvm2_pv::getVGName(partitionPath);

    if (partitionPath.isEmpty()) // Happens when during initial scan LUKS is closed
        return QString();

    QStringList mountPoints;
    QFileInfo partitionPathFileInfo(partitionPath);
    QString partitionCanonicalPath = partitionPathFileInfo.canonicalFilePath();
    const QList<QStorageInfo> mountedVolumes = QStorageInfo::mountedVolumes();
    for (const QStorageInfo &storage : mountedVolumes) {
        if (partitionCanonicalPath == QFileInfo(QFile::decodeName(storage.device())).canonicalFilePath() ) {
            mountPoints.append(storage.rootPath());
        }
    }

    mountPoints.append(possibleMountPoints(partitionPath));

    return mountPoints.isEmpty() ? QString() : mountPoints.first();
}

bool FileSystem::detectMountStatus(FileSystem* fs, const QString& partitionPath)
{
    bool mounted = false;

    if (fs->type() == FileSystem::Type::Lvm2_PV) {
        mounted = !FS::lvm2_pv::getVGName(partitionPath).isEmpty();
    } else {
        mounted = isMounted(partitionPath);
    }
    return mounted;
}

/** Reads the label for this FileSystem
    @param deviceNode the device node for the Partition the FileSystem is on
    @return the FileSystem label or an empty string in case of error
*/
QString FileSystem::readLabel(const QString& deviceNode) const
{
    return CoreBackendManager::self()->backend()->readLabel(deviceNode);
}

/** Creates a new FileSystem
    @param report Report to write status information to
    @param deviceNode the device node for the Partition to create the FileSystem on
    @return true if successful
*/
bool FileSystem::create(Report& report, const QString& deviceNode)
{
    Q_UNUSED(report)
    Q_UNUSED(deviceNode)

    return true;
}

/** Creates a new FileSystem with a specified Label
    @param report Report to write status information to
    @param deviceNode the device node for the Partition to create the FileSystem on
    @param label the new label for the FileSystem
    @return true if successful
*/
bool FileSystem::createWithLabel(Report& report, const QString& deviceNode, const QString& label)
{
    Q_UNUSED(report)
    Q_UNUSED(deviceNode)
    Q_UNUSED(label)

    return true;
}

/** Scans a new FileSystem and load file system specific class variables.
 *  @param deviceNode the device node for the Partition to create the FileSystem on
*/
void FileSystem::scan(const QString& deviceNode)
{
    Q_UNUSED(deviceNode)
}

/** Resize a FileSystem to a given new length
    @param report Report to write status information to
    @param deviceNode the device node for the Partition the FileSystem is on
    @param newLength the new length for the FileSystem in bytes
    @return true on success
*/
bool FileSystem::resize(Report& report, const QString& deviceNode, qint64 newLength) const
{
    Q_UNUSED(report)
    Q_UNUSED(deviceNode)
    Q_UNUSED(newLength)

    return true;
}

/** Resize a mounted FileSystem to a given new length
    @param report Report to write status information to
    @param deviceNode the device node for the Partition the FileSystem is on
    @param mountPoint the mount point where FileSystem is mounted on
    @param newLength the new length for the FileSystem in bytes
    @return true on success
*/
bool FileSystem::resizeOnline(Report& report, const QString& deviceNode, const QString& mountPoint, qint64 newLength) const
{
    Q_UNUSED(report)
    Q_UNUSED(deviceNode)
    Q_UNUSED(mountPoint)
    Q_UNUSED(newLength)

    return true;
}

/** Move a FileSystem to a new start sector
    @param report Report to write status information to
    @param deviceNode the device node for the Partition the FileSystem is on
    @param newStartSector the new start sector for the FileSystem
    @return true on success
*/
bool FileSystem::move(Report& report, const QString& deviceNode, qint64 newStartSector) const
{
    Q_UNUSED(report)
    Q_UNUSED(deviceNode)
    Q_UNUSED(newStartSector)

    return true;
}

/** Writes a label for the FileSystem to disk
    @param report Report to write status information to
    @param deviceNode the device node for the Partition the FileSystem is on
    @param newLabel the new label for the FileSystem
    @return true on success
*/
bool FileSystem::writeLabel(Report& report, const QString& deviceNode, const QString& newLabel)
{
    Q_UNUSED(report)
    Q_UNUSED(deviceNode)
    Q_UNUSED(newLabel)

    return true;
}

/** Writes a label for the FileSystem to disk
    @param report Report to write status information to
    @param deviceNode the device node for the Partition the FileSystem is on
    @param mountPoint the mount point where FileSystem is mounted on
    @param newLabel the new label for the FileSystem
    @return true on success
*/
bool FileSystem::writeLabelOnline(Report& report, const QString& deviceNode, const QString& mountPoint, const QString& newLabel)
{
    Q_UNUSED(report)
    Q_UNUSED(deviceNode)
    Q_UNUSED(mountPoint)
    Q_UNUSED(newLabel)

    return true;
}

/** Copies a FileSystem from one Partition to another
    @param report Report to write status information to
    @param targetDeviceNode device node of the target Partition
    @param sourceDeviceNode device node of the source Partition
    @return true on success
*/
bool FileSystem::copy(Report& report, const QString& targetDeviceNode, const QString& sourceDeviceNode) const
{
    Q_UNUSED(report)
    Q_UNUSED(targetDeviceNode)
    Q_UNUSED(sourceDeviceNode)

    return true;
}

/** Backs up a FileSystem to a file
    @param report Report to write status information to
    @param sourceDevice Device the source FileSystem is on
    @param deviceNode device node of the source Partition
    @param filename name of the file to backup to
    @return true on success
*/
bool FileSystem::backup(Report& report, const Device& sourceDevice, const QString& deviceNode, const QString& filename) const
{
    Q_UNUSED(report)
    Q_UNUSED(sourceDevice)
    Q_UNUSED(deviceNode)
    Q_UNUSED(filename)

    return false;
}

/** Removes a FileSystem
    @param report Report to write status information to
    @param deviceNode the device node for the Partition the FileSystem is on
    @return true if FileSystem is removed
*/
bool FileSystem::remove(Report& report, const QString& deviceNode) const
{
    Q_UNUSED(report)
    Q_UNUSED(deviceNode)

    return true;
}

/** Checks a FileSystem for errors
    @param report Report to write status information to
    @param deviceNode the device node for the Partition the FileSystem is on
    @return true if FileSystem is error-free
*/
bool FileSystem::check(Report& report, const QString& deviceNode) const
{
    Q_UNUSED(report)
    Q_UNUSED(deviceNode)

    return true;
}

/** Updates a FileSystem UUID on disk
    @param report Report to write status information to
    @param deviceNode the device node for the Partition the FileSystem is on
    @return true on success
*/
bool FileSystem::updateUUID(Report& report, const QString& deviceNode) const
{
    Q_UNUSED(report)
    Q_UNUSED(deviceNode)

    return true;
}

/** Returns the FileSystem UUID by calling a FileSystem-specific helper program
    @param deviceNode the device node for the Partition the FileSystem is on
    @return the UUID or an empty string if the FileSystem does not support UUIDs
 */
QString FileSystem::readUUID(const QString& deviceNode) const
{
    return CoreBackendManager::self()->backend()->readUUID(deviceNode);
}

/** Give implementations of FileSystem a chance to update the boot sector after the
    file system has been moved or copied.
    @param report Report to write status information to
    @param deviceNode the device node for the Partition the FileSystem is on
    @return true on success
*/
bool FileSystem::updateBootSector(Report& report, const QString& deviceNode) const
{
    Q_UNUSED(report)
    Q_UNUSED(deviceNode)

    return true;
}

/** @return the minimum capacity valid for this FileSystem in bytes */
qint64 FileSystem::minCapacity() const
{
    return 8 * Capacity::unitFactor(Capacity::Unit::Byte, Capacity::Unit::MiB);
}

/** @return the maximum capacity valid for this FileSystem in bytes */
qint64 FileSystem::maxCapacity() const
{
    return Capacity::unitFactor(Capacity::Unit::Byte, Capacity::Unit::EiB);
}

/** @return the maximum label length valid for this FileSystem */
int FileSystem::maxLabelLength() const
{
    return 16;
}

/** Validates the label for this FileSystem
 * @param parent the parent widget passed to the QObject constructor
 * @return QValidator to validate the file system label line edit input */
QValidator* FileSystem::labelValidator(QObject *parent) const
{
    Q_UNUSED(parent)
    return nullptr;
}

/** @return this FileSystem's type as printable name */
QString FileSystem::name(const QStringList& languages) const
{
    return nameForType(type(), languages);
}

FileSystem::Type FileSystem::type() const
{
    return d->m_Type;
}

/** @return a pointer to a QString C array with all FileSystem names */
static const KLocalizedString* typeNames()
{
    static const KLocalizedString s[] = {
        kxi18nc("@item filesystem name", "unknown"),
        kxi18nc("@item filesystem name", "extended"),

        kxi18nc("@item filesystem name", "ext2"),
        kxi18nc("@item filesystem name", "ext3"),
        kxi18nc("@item filesystem name", "ext4"),
        kxi18nc("@item filesystem name", "linuxswap"),
        kxi18nc("@item filesystem name", "freebsdswap"),
        kxi18nc("@item filesystem name", "fat16"),
        kxi18nc("@item filesystem name", "fat32"),
        kxi18nc("@item filesystem name", "ntfs"),
        kxi18nc("@item filesystem name", "reiser"),
        kxi18nc("@item filesystem name", "reiser4"),
        kxi18nc("@item filesystem name", "xfs"),
        kxi18nc("@item filesystem name", "jfs"),
        kxi18nc("@item filesystem name", "hfs"),
        kxi18nc("@item filesystem name", "hfsplus"),
        kxi18nc("@item filesystem name", "ufs"),
        kxi18nc("@item filesystem name", "unformatted"),
        kxi18nc("@item filesystem name", "btrfs"),
        kxi18nc("@item filesystem name", "hpfs"),
        kxi18nc("@item filesystem name", "luks"),
        kxi18nc("@item filesystem name", "ocfs2"),
        kxi18nc("@item filesystem name", "zfs"),
        kxi18nc("@item filesystem name", "exfat"),
        kxi18nc("@item filesystem name", "nilfs2"),
        kxi18nc("@item filesystem name", "lvm2 pv"),
        kxi18nc("@item filesystem name", "f2fs"),
        kxi18nc("@item filesystem name", "udf"),
        kxi18nc("@item filesystem name", "iso9660"),
        kxi18nc("@item filesystem name", "luks2"),
        kxi18nc("@item filesystem name", "fat12"),
        kxi18nc("@item filesystem name", "linux_raid_member"),
        kxi18nc("@item filesystem name", "BitLocker"),
        kxi18nc("@item filesystem name", "apfs"),
        kxi18nc("@item filesystem name", "minix"),
        kxi18nc("@item filesystem name", "bcachefs"),
    };

    return s;
}

/** @param t the type to get the name for
    @return the printable name for the given type
*/
QString FileSystem::nameForType(FileSystem::Type t, const QStringList& languages)
{
    Q_ASSERT(t < Type::__lastType);

    return typeNames()[static_cast<int>(t)].toString(languages);
}

/** @param s the name to get the type for
    @return the type for the name or FileSystem::Unknown if not found
*/
FileSystem::Type FileSystem::typeForName(const QString& s, const QStringList& languages )
{
    for (quint32 i = 0; i < static_cast<int>(Type::__lastType); i++)
        if (typeNames()[i].toString(languages) == s)
            return static_cast<FileSystem::Type>(i);

    return Type::Unknown;
}

/** @return a QList of all known types */
QList<FileSystem::Type> FileSystem::types()
{
    QList<FileSystem::Type> result;

    int i = static_cast<int>(Type::Ext2); // first "real" filesystem
    while (i != static_cast<int>(Type::__lastType))
        result.append(static_cast<FileSystem::Type>(i++));

    return result;
}

/** @return printable menu title for mounting this FileSystem */
QString FileSystem::mountTitle() const
{
    return xi18nc("@title:menu", "Mount");
}

/** @return printable menu title for unmounting this FileSystem */
QString FileSystem::unmountTitle() const
{
    return xi18nc("@title:menu", "Unmount");
}

/** Moves a FileSystem to a new start sector.
    @param newStartSector where the FileSystem should be moved to
*/
void FileSystem::move(qint64 newStartSector)
{
    const qint64 savedLength = length();
    setFirstSector(newStartSector);
    setLastSector(newStartSector + savedLength - 1);
}
bool FileSystem::canMount(const QString& deviceNode, const QString& mountPoint) const
{
    Q_UNUSED(deviceNode)
    // cannot mount if we have no mount points
    return !mountPoint.isEmpty();
}

/** Attempt to mount this FileSystem on a given mount point
    @param report the report to write information to
    @param deviceNode the path to the device that is to be unmounted
    @param mountPoint the mount point to mount the FileSystem on
    @return true on success
*/
bool FileSystem::mount(Report& report, const QString &deviceNode, const QString &mountPoint)
{
    ExternalCommand mountCmd(   report,
                                QStringLiteral("mount"),
                              { QStringLiteral("--verbose"),
                                deviceNode,
                                mountPoint });
    if (mountCmd.run() && mountCmd.exitCode() == 0) {
        return true;
    }
    return false;
}

/** Attempt to unmount this FileSystem
    @param report the report to write information to
    @param deviceNode the path to the device that is to be unmounted
    @return true on success
 */
bool FileSystem::unmount(Report& report, const QString& deviceNode)
{
    ExternalCommand umountCmd(  report,
                                QStringLiteral("umount"),
                              { QStringLiteral("--verbose"),
                                QStringLiteral("--all-targets"),
                                deviceNode });
    if ( umountCmd.run() && umountCmd.exitCode() == 0 )
        return true;
    return false;
}

qint64 FileSystem::firstSector() const
{
    return d->m_FirstSector;
}

qint64 FileSystem::lastSector() const
{
        return d->m_LastSector;
}

bool FileSystem::findExternal(const QString& cmdName, const QStringList& args, int expectedCode)
{
    QString cmdFullPath = findTrustedCommand(cmdName);

    if (cmdFullPath.isEmpty())
        return false;

    ExternalCommand cmd(cmdFullPath, args);
    if (!cmd.run())
        return false;

    return cmd.exitCode() == 0 || cmd.exitCode() == expectedCode;
}

void FileSystem::addAvailableFeature(const QString& name)
{
    d->m_AvailableFeatures.append(name);
}

void FileSystem::addFeature(const QString& name, const QVariant& value)
{
    d->m_Features.insert(name, value);
}

void FileSystem::addFeatures(const QVariantMap& features)
{
    for (const auto& k : features.keys()) {
        d->m_Features.insert(k, features.value(k));
    }
}

bool FileSystem::supportToolFound() const
{
    return false;
}

FileSystem::SupportTool FileSystem::supportToolName() const
{
    return SupportTool();
}

void FileSystem::setFirstSector(qint64 s)
{
    d->m_FirstSector = s;
}

void FileSystem::setLastSector(qint64 s)
{
    d->m_LastSector = s;
}

const QString& FileSystem::label() const
{
    return d->m_Label;
}

const QStringList& FileSystem::availableFeatures() const
{
    return d->m_AvailableFeatures;
}

const QVariantMap& FileSystem::features() const
{
    return d->m_Features;
}

qint64 FileSystem::sectorSize() const
{
    return d->m_SectorSize;
}

qint64 FileSystem::sectorsUsed() const
{
    return d->m_SectorsUsed;
}

const QString& FileSystem::uuid() const
{
    return d->m_UUID;
}

void FileSystem::setSectorSize(qint64 s)
{
    d->m_SectorSize = s;
}

void FileSystem::setSectorsUsed(qint64 s)
{
    d->m_SectorsUsed = s;
}

void FileSystem::setLabel(const QString& s)
{
    d->m_Label = s;
}

void FileSystem::setUUID(const QString& s)
{
    d->m_UUID = s;
}
