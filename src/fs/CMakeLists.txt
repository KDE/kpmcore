# SPDX-FileCopyrightText: 2008,2012 Volker Lanz <vl@fidra.de>
# SPDX-FileCopyrightText: 2015-2019 Andrius Štikonas <andrius@stikonas.eu>
# SPDX-FileCopyrightText: 2015 Chris Campbell <c.j.campbell@ed.ac.uk>
# SPDX-FileCopyrightText: 2015 Teo Mrnjavac <teo@kde.org>
# SPDX-FileCopyrightText: 2017 Pali Rohár <pali.rohar@gmail.com>
# SPDX-FileCopyrightText: 2018 Caio Jordão Carvalho <caiojcarvalho@gmail.com>
# SPDX-FileCopyrightText: 2019 Shubham Jangra <aryan100jangid@gmail.com>
# SPDX-FileCopyrightText: 2020 Arnaud Ferraris <arnaud.ferraris@collabora.com>
# SPDX-FileCopyrightText: 2020 Gaël PORTAY <gael.portay@collabora.com>

# SPDX-License-Identifier: GPL-3.0-or-later

set(FS_SRC
    fs/apfs.cpp
    fs/bitlocker.cpp
    fs/btrfs.cpp
    fs/exfat.cpp
    fs/ext2.cpp
    fs/ext3.cpp
    fs/ext4.cpp
    fs/extended.cpp
    fs/f2fs.cpp
    fs/fat12.cpp
    fs/fat16.cpp
    fs/fat32.cpp
    fs/filesystem.cpp
    fs/filesystemfactory.cpp
    fs/hfs.cpp
    fs/hfsplus.cpp
    fs/hpfs.cpp
    fs/iso9660.cpp
    fs/jfs.cpp
    fs/linuxraidmember.cpp
    fs/linuxswap.cpp
    fs/luks.cpp
    fs/luks2.cpp
    fs/lvm2_pv.cpp
    fs/minix.cpp
    fs/nilfs2.cpp
    fs/ntfs.cpp
    fs/ocfs2.cpp
    fs/reiser4.cpp
    fs/reiserfs.cpp
    fs/udf.cpp
    fs/ufs.cpp
    fs/unformatted.cpp
    fs/unknown.cpp
    fs/xfs.cpp
    fs/zfs.cpp
)

set(FS_LIB_HDRS
    fs/apfs.h
    fs/bitlocker.h
    fs/btrfs.h
    fs/exfat.h
    fs/ext2.h
    fs/ext3.h
    fs/ext4.h
    fs/extended.h
    fs/f2fs.h
    fs/fat12.h
    fs/fat16.h
    fs/fat32.h
    fs/filesystem.h
    fs/filesystemfactory.h
    fs/hfs.h
    fs/hfsplus.h
    fs/hpfs.h
    fs/iso9660.h
    fs/jfs.h
    fs/linuxraidmember.h
    fs/linuxswap.h
    fs/luks.h
    fs/luks2.h
    fs/lvm2_pv.h
    fs/minix.h
    fs/nilfs2.h
    fs/ntfs.h
    fs/ocfs2.h
    fs/reiser4.h
    fs/reiserfs.h
    fs/udf.h
    fs/ufs.h
    fs/unformatted.h
    fs/unknown.h
    fs/xfs.h
    fs/zfs.h
)
