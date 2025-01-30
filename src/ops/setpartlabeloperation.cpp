/*
    SPDX-FileCopyrightText: 2008-2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2025 Eugene Shalygin <eugene.shalygin@gmail.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "ops/setpartlabeloperation.h"

#include "core/device.h"
#include "core/partition.h"
#include "core/partitionnode.h"
#include "core/partitiontable.h"
#include "fs/filesystem.h"
#include "jobs/setpartitionlabeljob.h"

#include <QString>

#include <KLocalizedString>

/** Creates a new SetPartLabelOperation.
    @param d the Device on which the Partition to set flags for is
    @param p the Partition to set new flags for
    @param label the new label to set
*/
SetPartLabelOperation::SetPartLabelOperation(Device& d, Partition& p, const QString& label) :
    Operation(),
    m_TargetDevice(d),
    m_partition(p),
    m_OldLabel(partition().label()),
    m_NewLabel(label),
    m_LabelJob(new SetPartitionLabelJob(targetDevice(), partition(), newLabel()))
{
    addJob(labelJob());
}

bool SetPartLabelOperation::targets(const Device& d) const
{
    return d == targetDevice();
}

bool SetPartLabelOperation::targets(const Partition& p) const
{
    return p == partition();
}

void SetPartLabelOperation::preview()
{
    partition().setLabel(newLabel());
}

void SetPartLabelOperation::undo()
{
    partition().setLabel(oldLabel());
}

QString SetPartLabelOperation::description() const
{
    return xi18nc("@info:status", "Set GPT partition label <filename>%1</filename> to \"%2\"", partition().deviceNode(), newLabel());
}
