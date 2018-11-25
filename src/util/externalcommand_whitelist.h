/*************************************************************************
 *  Copyright (C) 2018 by Andrius Štikonas <andrius@stikonas.eu>         *
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

#ifndef KPMCORE_EXTERNALCOMMAND_WHITELIST_H
#define KPMCORE_EXTERNALCOMMAND_WHITELIST_H

QString allowedCommands[] = {
// TODO try to remove these later
QStringLiteral("mv"),

// TODO no root needed
QStringLiteral("lsblk"),
QStringLiteral("udevadm"),

//Core programs
QStringLiteral("blockdev"),
QStringLiteral("sfdisk"),
QStringLiteral("wipefs"),
QStringLiteral("lvm"),
QStringLiteral("mdadm"),
QStringLiteral("mount"),
QStringLiteral("umount"),
QStringLiteral("smartctl"),

// FileSystem utilties
QStringLiteral("btrfs"),
QStringLiteral("mkfs.btrfs"),
QStringLiteral("btrfstune"),
QStringLiteral("exfatfsck"),
QStringLiteral("mkfs.exfat"),
QStringLiteral("exfatlabel"),
QStringLiteral("dumpe2fs"),
QStringLiteral("e2fsck"),
QStringLiteral("mkfs.ext2"),
QStringLiteral("resize2fs"),
QStringLiteral("e2label"),
QStringLiteral("tune2fs"),
QStringLiteral("mkfs.ext3"),
QStringLiteral("mkfs.ext4"),
QStringLiteral("mkfs.f2fs"),
QStringLiteral("fsck.f2fs"),
QStringLiteral("resize.f2fs"),
QStringLiteral("fsck.fat"),
QStringLiteral("fatlabel"),
QStringLiteral("mkfs.fat"),
QStringLiteral("fatresize"),
QStringLiteral("hfsck"),
QStringLiteral("hformat"),
QStringLiteral("fsck.hfsplus"),
QStringLiteral("mkfs.hfsplus"),
QStringLiteral("jfs_debugfs"),
QStringLiteral("jfs_tune"),
QStringLiteral("fsck.jfs"),
QStringLiteral("mkfs.jfs"),
QStringLiteral("mkswap"),
QStringLiteral("swaplabel"),
QStringLiteral("swapon"),
QStringLiteral("swapoff"),
QStringLiteral("cryptsetup"),
QStringLiteral("dmsetup"),
QStringLiteral("fsck.nilfs2"),
QStringLiteral("mkfs.nilfs2"),
QStringLiteral("nilfs-tune"),
QStringLiteral("nilfs-resize"),
QStringLiteral("ntfsresize"),
QStringLiteral("mkfs.ntfs"),
QStringLiteral("ntfsclone"),
QStringLiteral("ntfslabel"),
QStringLiteral("fsck.ocfs2"),
QStringLiteral("mkfs.ocfs2"),
QStringLiteral("debugfs.ocfs2"),
QStringLiteral("tunefs.ocfs2"),
QStringLiteral("debugfs.reiser4"),
QStringLiteral("fsck.reiser4"),
QStringLiteral("mkfs.reiser4"),
QStringLiteral("debugreiserfs"),
QStringLiteral("reiserfstune"),
QStringLiteral("fsck.reiserfs"),
QStringLiteral("mkfs.reiserfs"),
QStringLiteral("resize_reiserfs"),
QStringLiteral("mkudffs"),
QStringLiteral("udfinfo"),
QStringLiteral("udflabel"),
QStringLiteral("xfs_db"),
QStringLiteral("xfs_repair"),
QStringLiteral("mkfs.xfs"),
QStringLiteral("xfs_copy"),
QStringLiteral("xfs_growfs"),
QStringLiteral("zpool")
};

#endif
