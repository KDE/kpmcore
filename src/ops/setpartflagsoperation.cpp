/*
    SPDX-FileCopyrightText: 2008-2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2014-2016 Andrius Štikonas <andrius@stikonas.eu>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "ops/setpartflagsoperation.h"

#include "core/partition.h"
#include "core/partitionnode.h"
#include "core/partitiontable.h"
#include "core/device.h"

#include "jobs/setpartflagsjob.h"

#include "fs/filesystem.h"

#include <QString>

#include <KLocalizedString>

/** Creates a new SetPartFlagsOperation.
    @param d the Device on which the Partition to set flags for is
    @param p the Partition to set new flags for
    @param flags the new flags to set
*/
SetPartFlagsOperation::SetPartFlagsOperation(Device& d, Partition& p, const PartitionTable::Flags& flags) :
    Operation(),
    m_TargetDevice(d),
    m_FlagPartition(p),
    m_OldFlags(flagPartition().activeFlags()),
    m_NewFlags(flags),
    m_FlagsJob(new SetPartFlagsJob(targetDevice(), flagPartition(), newFlags()))
{
    addJob(flagsJob());
}

bool SetPartFlagsOperation::targets(const Device& d) const
{
    return d == targetDevice();
}

bool SetPartFlagsOperation::targets(const Partition& p) const
{
    return p == flagPartition();
}

void SetPartFlagsOperation::preview()
{
    flagPartition().setFlags(newFlags());
}

void SetPartFlagsOperation::undo()
{
    flagPartition().setFlags(oldFlags());
}

QString SetPartFlagsOperation::description() const
{
    if (PartitionTable::flagNames(newFlags()).size() == 0)
        return xi18nc("@info:status", "Clear flags for partition <filename>%1</filename>", flagPartition().deviceNode());

    return xi18nc("@info:status", "Set flags for partition <filename>%1</filename> to \"%2\"", flagPartition().deviceNode(), PartitionTable::flagNames(newFlags()).join(QStringLiteral(",")));
}
