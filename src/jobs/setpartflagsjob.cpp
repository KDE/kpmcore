/*
    SPDX-FileCopyrightText: 2008-2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2014-2020 Andrius Štikonas <andrius@stikonas.eu>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "jobs/setpartflagsjob.h"

#include "backend/corebackend.h"
#include "backend/corebackendmanager.h"
#include "backend/corebackenddevice.h"
#include "backend/corebackendpartitiontable.h"

#include "core/device.h"
#include "core/partition.h"
#include "core/partitionrole.h"
#include "core/partitiontable.h"

#include "util/report.h"

#include <KLocalizedString>

/** Creates a new SetPartFlagsJob
    @param d the Device the Partition whose flags are to be set is on
    @param p the Partition whose flags are to be set
    @param flags the new flags for the Partition
*/
SetPartFlagsJob::SetPartFlagsJob(Device& d, Partition& p, PartitionTable::Flags flags) :
    Job(),
    m_Device(d),
    m_Partition(p),
    m_Flags(flags)
{
}

qint32 SetPartFlagsJob::numSteps() const
{
    return PartitionTable::flagList().size();
}

bool SetPartFlagsJob::run(Report& parent)
{
    bool rval = true;

    Report* report = jobStarted(parent);

    std::unique_ptr<CoreBackendDevice> backendDevice = CoreBackendManager::self()->backend()->openDevice(device());

    if (backendDevice) {
        std::unique_ptr<CoreBackendPartitionTable> backendPartitionTable = backendDevice->openPartitionTable();

        if (backendPartitionTable) {
            int count = 0;

            for (const auto &f : PartitionTable::flagList()) {
                Q_EMIT progress(++count);

                const bool oldState = (partition().activeFlags() & f) ? true : false;
                const bool state = (flags() & f) ? true : false;
                if (oldState == state)
                        continue;

                if (!backendPartitionTable->setFlag(*report, partition(), f, state)) {
                    report->line() << xi18nc("@info:progress", "There was an error setting flag %1 for partition <filename>%2</filename> to state %3.", PartitionTable::flagName(f), partition().deviceNode(), state ? xi18nc("@info:progress flag turned on, active", "on") : xi18nc("@info:progress flag turned off, inactive", "off"));

                    rval = false;
                }
            }

            if (rval)
                backendPartitionTable->commit();
        } else
            report->line() << xi18nc("@info:progress", "Could not open partition table on device <filename>%1</filename> to set partition flags for partition <filename>%2</filename>.", device().deviceNode(), partition().deviceNode());
    } else
        report->line() << xi18nc("@info:progress", "Could not open device <filename>%1</filename> to set partition flags for partition <filename>%2</filename>.", device().deviceNode(), partition().deviceNode());

    if (rval)
        partition().setFlags(flags());

    jobFinished(*report, rval);

    return rval;
}

QString SetPartFlagsJob::description() const
{
    if (PartitionTable::flagNames(flags()).size() == 0)
        return xi18nc("@info:progress", "Clear flags for partition <filename>%1</filename>", partition().deviceNode());

    return xi18nc("@info:progress", "Set the flags for partition <filename>%1</filename> to \"%2\"", partition().deviceNode(), PartitionTable::flagNames(flags()).join(QStringLiteral(",")));
}
