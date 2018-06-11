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

#if !defined(KPMCORE_LIBPARTED_H)

#define KPMCORE_LIBPARTED_H

#include "backend/corebackend.h"

#include "core/partitiontable.h"
#include "core/diskdevice.h"
#include "util/libpartitionmanagerexport.h"

#include "fs/filesystem.h"

#include <parted/parted.h>

#include <QList>
#include <QVariant>
#include <QtGlobal>

class LibPartedDevice;
class LibPartedPartitionTable;
class LibPartedPartition;
class OperationStack;

class KPluginFactory;
class QString;

/** Backend plugin for libparted.

    @author Volker Lanz <vl@fidra.de>
*/
class LibPartedBackend : public CoreBackend
{
    friend class KPluginFactory;
    friend class LibPartedPartition;
    friend class LibPartedDevice;
    friend class LibPartedPartitionTable;

    Q_DISABLE_COPY(LibPartedBackend)

private:
    LibPartedBackend(QObject* parent, const QList<QVariant>& args);

public:
    void initFSSupport() override;

    CoreBackendDevice* openDevice(const Device& d) override;
    CoreBackendDevice* openDeviceExclusive(const Device& d) override;
    bool closeDevice(CoreBackendDevice* core_device) override;
    Device* scanDevice(const QString& deviceNode) override;
    QList<Device*> scanDevices(bool excludeReadOnly = false) override;
    FileSystem::Type detectFileSystem(const QString& partitionPath) override;
    QString readLabel(const QString& deviceNode) const override;
    QString readUUID(const QString& deviceNode) const override;

    static QString lastPartedExceptionMessage();

private:
    static PedPartitionFlag getPedFlag(PartitionTable::Flag flag);
    void scanDevicePartitions(Device& d, PedDisk* pedDisk);
};

#endif
