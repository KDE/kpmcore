/*
    SPDX-FileCopyrightText: 2008-2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2014-2016 Andrius Štikonas <andrius@stikonas.eu>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "ops/setfilesystemlabeloperation.h"

#include "core/partition.h"
#include "core/device.h"

#include "jobs/setfilesystemlabeljob.h"

#include "fs/filesystem.h"

#include <QString>

#include <KLocalizedString>

/** Creates a new SetFileSystemLabelOperation.
    @param p the Partition with the FileSystem to set the label for
    @param newlabel the new label
*/
SetFileSystemLabelOperation::SetFileSystemLabelOperation(Partition& p, const QString& newlabel) :
    Operation(),
    m_LabeledPartition(p),
    m_OldLabel(labeledPartition().fileSystem().label()),
    m_NewLabel(newlabel),
    m_LabelJob(new SetFileSystemLabelJob(labeledPartition(), newLabel()))
{
    addJob(labelJob());
}

bool SetFileSystemLabelOperation::targets(const Device& d) const
{
    return labeledPartition().devicePath() == d.deviceNode();
}

bool SetFileSystemLabelOperation::targets(const Partition& p) const
{
    return p == labeledPartition();
}

void SetFileSystemLabelOperation::preview()
{
    labeledPartition().fileSystem().setLabel(newLabel());
}

void SetFileSystemLabelOperation::undo()
{
    labeledPartition().fileSystem().setLabel(oldLabel());
}

QString SetFileSystemLabelOperation::description() const
{
    if (oldLabel().isEmpty())
        return xi18nc("@info:status", "Set label for partition <filename>%1</filename> to \"%2\"", labeledPartition().deviceNode(), newLabel());

    return xi18nc("@info:status", "Set label for partition <filename>%1</filename> from \"%2\" to \"%3\"", labeledPartition().deviceNode(), oldLabel(), newLabel());
}
