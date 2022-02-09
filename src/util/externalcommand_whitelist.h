/*
    SPDX-FileCopyrightText: 2018-2020 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2019 Shubham Jangra <aryan100jangid@gmail.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KPMCORE_EXTERNALCOMMAND_WHITELIST_H
#define KPMCORE_EXTERNALCOMMAND_WHITELIST_H

#include <unordered_set>
#include "util/externalcommand_trustedprefixes.h"

const std::unordered_set<QString> allowedCommands {
// TODO no root needed
QStringLiteral("lsblk"),
QStringLiteral("udevadm"),

//Core programs
QStringLiteral("blockdev"),
QStringLiteral("blkid"),
QStringLiteral("chmod"),
QStringLiteral("partx"),
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
QStringLiteral("fsck.exfat"),
QStringLiteral("mkexfatfs"),
QStringLiteral("mkfs.exfat"),
QStringLiteral("exfatlabel"),
QStringLiteral("tune.exfat"),
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
QStringLiteral("fsck.minix"),
QStringLiteral("mkfs.minix"),
QStringLiteral("fsck.nilfs2"),
QStringLiteral("mkfs.nilfs2"),
QStringLiteral("nilfs-tune"),
QStringLiteral("nilfs-resize"),
QStringLiteral("ntfsresize"),
QStringLiteral("mkfs.ntfs"),
QStringLiteral("ntfsclone"),
QStringLiteral("ntfsinfo"),
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
