/*************************************************************************
 *  Copyright (C) 2012 by Volker Lanz <vl@fidra.de>                      *
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

#include "fs/filesystemfactory.h"
#include "fs/filesystem.h"

#include "fs/btrfs.h"
#include "fs/exfat.h"
#include "fs/ext2.h"
#include "fs/ext3.h"
#include "fs/ext4.h"
#include "fs/extended.h"
#include "fs/f2fs.h"
#include "fs/fat12.h"
#include "fs/fat16.h"
#include "fs/fat32.h"
#include "fs/hfs.h"
#include "fs/hfsplus.h"
#include "fs/hpfs.h"
#include "fs/iso9660.h"
#include "fs/jfs.h"
#include "fs/linuxraidmember.h"
#include "fs/linuxswap.h"
#include "fs/luks.h"
#include "fs/luks2.h"
#include "fs/lvm2_pv.h"
#include "fs/nilfs2.h"
#include "fs/ntfs.h"
#include "fs/ocfs2.h"
#include "fs/reiser4.h"
#include "fs/reiserfs.h"
#include "fs/udf.h"
#include "fs/ufs.h"
#include "fs/unformatted.h"
#include "fs/unknown.h"
#include "fs/xfs.h"
#include "fs/zfs.h"

#include "backend/corebackendmanager.h"
#include "backend/corebackend.h"

FileSystemFactory::FileSystems FileSystemFactory::m_FileSystems;

/** Initializes the instance. */
void FileSystemFactory::init()
{
    qDeleteAll(m_FileSystems);
    m_FileSystems.clear();

    m_FileSystems.insert(FileSystem::Type::Btrfs, new FS::btrfs(-1, -1, -1, QString()));
    m_FileSystems.insert(FileSystem::Type::Exfat, new FS::exfat(-1, -1, -1, QString()));
    m_FileSystems.insert(FileSystem::Type::Ext2, new FS::ext2(-1, -1, -1, QString()));
    m_FileSystems.insert(FileSystem::Type::Ext3, new FS::ext3(-1, -1, -1, QString()));
    m_FileSystems.insert(FileSystem::Type::Ext4, new FS::ext4(-1, -1, -1, QString()));
    m_FileSystems.insert(FileSystem::Type::Extended, new FS::extended(-1, -1, -1, QString()));
    m_FileSystems.insert(FileSystem::Type::F2fs, new FS::f2fs(-1, -1, -1, QString()));
    m_FileSystems.insert(FileSystem::Type::Fat12, new FS::fat12(-1, -1, -1, QString()));
    m_FileSystems.insert(FileSystem::Type::Fat16, new FS::fat16(-1, -1, -1, QString()));
    m_FileSystems.insert(FileSystem::Type::Fat32, new FS::fat32(-1, -1, -1, QString()));
    m_FileSystems.insert(FileSystem::Type::Hfs, new FS::hfs(-1, -1, -1, QString()));
    m_FileSystems.insert(FileSystem::Type::HfsPlus, new FS::hfsplus(-1, -1, -1, QString()));
    m_FileSystems.insert(FileSystem::Type::Hpfs, new FS::hpfs(-1, -1, -1, QString()));
    m_FileSystems.insert(FileSystem::Type::Iso9660, new FS::iso9660(-1, -1, -1, QString()));
    m_FileSystems.insert(FileSystem::Type::Jfs, new FS::jfs(-1, -1, -1, QString()));
    m_FileSystems.insert(FileSystem::Type::LinuxRaidMember, new FS::linuxraidmember(-1, -1, -1, QString()));
    m_FileSystems.insert(FileSystem::Type::LinuxSwap, new FS::linuxswap(-1, -1, -1, QString()));
    m_FileSystems.insert(FileSystem::Type::Luks, new FS::luks(-1, -1, -1, QString()));
    m_FileSystems.insert(FileSystem::Type::Luks2, new FS::luks2(-1, -1, -1, QString()));
    m_FileSystems.insert(FileSystem::Type::Lvm2_PV, new FS::lvm2_pv(-1, -1, -1, QString()));
    m_FileSystems.insert(FileSystem::Type::Nilfs2, new FS::nilfs2(-1, -1, -1, QString()));
    m_FileSystems.insert(FileSystem::Type::Ntfs, new FS::ntfs(-1, -1, -1, QString()));
    m_FileSystems.insert(FileSystem::Type::Ocfs2, new FS::ocfs2(-1, -1, -1, QString()));
    m_FileSystems.insert(FileSystem::Type::ReiserFS, new FS::reiserfs(-1, -1, -1, QString()));
    m_FileSystems.insert(FileSystem::Type::Reiser4, new FS::reiser4(-1, -1, -1, QString()));
    m_FileSystems.insert(FileSystem::Type::Udf, new FS::udf(-1, -1, -1, QString()));
    m_FileSystems.insert(FileSystem::Type::Ufs, new FS::ufs(-1, -1, -1, QString()));
    m_FileSystems.insert(FileSystem::Type::Unformatted, new FS::unformatted(-1, -1, -1, QString()));
    m_FileSystems.insert(FileSystem::Type::Unknown, new FS::unknown(-1, -1, -1, QString()));
    m_FileSystems.insert(FileSystem::Type::Xfs, new FS::xfs(-1, -1, -1, QString()));
    m_FileSystems.insert(FileSystem::Type::Zfs, new FS::zfs(-1, -1, -1, QString()));

    for (const auto &fs : FileSystemFactory::map())
        fs->init();

    CoreBackendManager::self()->backend()->initFSSupport();
}

/** Creates a new FileSystem
    @param t the FileSystem's type
    @param firstsector the FileSystem's first sector relative to the Device
    @param lastsector the FileSystem's last sector relative to the Device
    @param sectorsused the number of used sectors in the FileSystem
    @param label the FileSystem's label
    @return pointer to the newly created FileSystem object or nullptr if FileSystem could not be created
*/
FileSystem* FileSystemFactory::create(FileSystem::Type t, qint64 firstsector, qint64 lastsector, qint64 sectorSize, qint64 sectorsused, const QString& label, const QString& uuid)
{
    FileSystem* fs = nullptr;

    switch (t) {
    case FileSystem::Type::Btrfs:           fs = new FS::btrfs(firstsector, lastsector, sectorsused, label); break;
    case FileSystem::Type::Exfat:           fs = new FS::exfat(firstsector, lastsector, sectorsused, label); break;
    case FileSystem::Type::Ext2:            fs = new FS::ext2(firstsector, lastsector, sectorsused, label); break;
    case FileSystem::Type::Ext3:            fs = new FS::ext3(firstsector, lastsector, sectorsused, label); break;
    case FileSystem::Type::Ext4:            fs = new FS::ext4(firstsector, lastsector, sectorsused, label); break;
    case FileSystem::Type::Extended:        fs = new FS::extended(firstsector, lastsector, sectorsused, label); break;
    case FileSystem::Type::F2fs:            fs = new FS::f2fs(firstsector, lastsector, sectorsused, label); break;
    case FileSystem::Type::Fat12:           fs = new FS::fat12(firstsector, lastsector, sectorsused, label); break;
    case FileSystem::Type::Fat16:           fs = new FS::fat16(firstsector, lastsector, sectorsused, label); break;
    case FileSystem::Type::Fat32:           fs = new FS::fat32(firstsector, lastsector, sectorsused, label); break;
    case FileSystem::Type::Hfs:             fs = new FS::hfs(firstsector, lastsector, sectorsused, label); break;
    case FileSystem::Type::HfsPlus:         fs = new FS::hfsplus(firstsector, lastsector, sectorsused, label); break;
    case FileSystem::Type::Hpfs:            fs = new FS::hpfs(firstsector, lastsector, sectorsused, label); break;
    case FileSystem::Type::Iso9660:         fs = new FS::iso9660(firstsector, lastsector, sectorsused, label); break;
    case FileSystem::Type::Jfs:             fs = new FS::jfs(firstsector, lastsector, sectorsused, label); break;
    case FileSystem::Type::LinuxRaidMember: fs = new FS::linuxraidmember(firstsector, lastsector, sectorsused, label); break;
    case FileSystem::Type::LinuxSwap:       fs = new FS::linuxswap(firstsector, lastsector, sectorsused, label); break;
    case FileSystem::Type::Luks:            fs = new FS::luks(firstsector, lastsector, sectorsused, label); break;
    case FileSystem::Type::Luks2:           fs = new FS::luks2(firstsector, lastsector, sectorsused, label); break;
    case FileSystem::Type::Lvm2_PV:         fs = new FS::lvm2_pv(firstsector, lastsector, sectorsused, label); break;
    case FileSystem::Type::Nilfs2:          fs = new FS::nilfs2(firstsector, lastsector, sectorsused, label); break;
    case FileSystem::Type::Ntfs:            fs = new FS::ntfs(firstsector, lastsector, sectorsused, label); break;
    case FileSystem::Type::Ocfs2:           fs = new FS::ocfs2(firstsector, lastsector, sectorsused, label); break;
    case FileSystem::Type::ReiserFS:        fs = new FS::reiserfs(firstsector, lastsector, sectorsused, label); break;
    case FileSystem::Type::Reiser4:         fs = new FS::reiser4(firstsector, lastsector, sectorsused, label); break;
    case FileSystem::Type::Udf:             fs = new FS::udf(firstsector, lastsector, sectorsused, label); break;
    case FileSystem::Type::Ufs:             fs = new FS::ufs(firstsector, lastsector, sectorsused, label); break;
    case FileSystem::Type::Unformatted:     fs = new FS::unformatted(firstsector, lastsector, sectorsused, label); break;
    case FileSystem::Type::Unknown:         fs = new FS::unknown(firstsector, lastsector, sectorsused, label); break;
    case FileSystem::Type::Xfs:             fs = new FS::xfs(firstsector, lastsector, sectorsused, label); break;
    case FileSystem::Type::Zfs:             fs = new FS::zfs(firstsector, lastsector, sectorsused, label); break;
    default:                       break;
    }

    if (fs != nullptr) {
        fs->setUUID(uuid);
        fs->setSectorSize(sectorSize);
    }

    return fs;
}

/**
    @overload
*/
FileSystem* FileSystemFactory::create(const FileSystem& other)
{
    return create(other.type(), other.firstSector(), other.lastSector(), other.sectorSize(), other.sectorsUsed(), other.label(), other.uuid());
}

/** @return the map of FileSystems */
const FileSystemFactory::FileSystems& FileSystemFactory::map()
{
    return m_FileSystems;
}

/** Clones a FileSystem from another one, but with a new type.
    @param newType the new FileSystem's type
    @param other the old FileSystem to clone
    @return pointer to the newly created FileSystem or nullptr in case of errors
*/
FileSystem* FileSystemFactory::cloneWithNewType(FileSystem::Type newType, const FileSystem& other)
{
    return create(newType, other.firstSector(), other.lastSector(), other.sectorSize(), other.sectorsUsed(), other.label());
}
