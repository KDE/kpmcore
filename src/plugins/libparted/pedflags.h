/*************************************************************************
 *  Copyright (C) 2008, 2010 by Volker Lanz <vl@fidra.de>                *
 *  Copyright (C) 2015 by Teo Mrnjavac <teo@kde.org>                     *
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

#ifndef KPMCORE_PEDFLAGS_H
#define KPMCORE_PEDFLAGS_H

static struct {
    PedPartitionFlag pedFlag;
    PartitionTable::Flag flag;
} flagmap[] = {
    { PED_PARTITION_BOOT,               PartitionTable::FlagBoot },
    { PED_PARTITION_ROOT,               PartitionTable::FlagRoot },
    { PED_PARTITION_SWAP,               PartitionTable::FlagSwap },
    { PED_PARTITION_HIDDEN,             PartitionTable::FlagHidden },
    { PED_PARTITION_RAID,               PartitionTable::FlagRaid },
    { PED_PARTITION_LVM,                PartitionTable::FlagLvm },
    { PED_PARTITION_LBA,                PartitionTable::FlagLba },
    { PED_PARTITION_HPSERVICE,          PartitionTable::FlagHpService },
    { PED_PARTITION_PALO,               PartitionTable::FlagPalo },
    { PED_PARTITION_PREP,               PartitionTable::FlagPrep },
    { PED_PARTITION_MSFT_RESERVED,      PartitionTable::FlagMsftReserved },
    { PED_PARTITION_BIOS_GRUB,          PartitionTable::FlagBiosGrub },
    { PED_PARTITION_APPLE_TV_RECOVERY,  PartitionTable::FlagAppleTvRecovery },
    { PED_PARTITION_DIAG,               PartitionTable::FlagDiag }, // generic diagnostics flag
    { PED_PARTITION_LEGACY_BOOT,        PartitionTable::FlagLegacyBoot },
    { PED_PARTITION_MSFT_DATA,          PartitionTable::FlagMsftData },
    { PED_PARTITION_IRST,               PartitionTable::FlagIrst }, // Intel Rapid Start partition
    { PED_PARTITION_ESP,                PartitionTable::FlagEsp }   // EFI system
};

#endif
