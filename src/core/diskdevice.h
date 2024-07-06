/*
    SPDX-FileCopyrightText: 2008-2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2012-2018 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2016 Chantara Tith <tith.chantara@gmail.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KPMCORE_DISKDEVICE_H
#define KPMCORE_DISKDEVICE_H

#include "util/libpartitionmanagerexport.h"
#include "core/device.h"

#include <memory>

#include <QString>
#include <QObject>
#include <QtGlobal>

class PartitionTable;
class CreatePartitionTableOperation;
class CoreBackend;
class SmartStatus;
class DiskDevicePrivate;

/** A disk device.

    Represents a device like /dev/sda.

    Devices are the outermost entity; they contain a PartitionTable that itself contains Partitions.

    @see PartitionTable, Partition
    @author Volker Lanz <vl@fidra.de>
*/

class LIBKPMCORE_EXPORT DiskDevice : public Device
{
    Q_DISABLE_COPY(DiskDevice)

    friend class CreatePartitionTableOperation;
    friend class CoreBackend;

public:
    DiskDevice(const QString& name, const QString& deviceNode, qint64 sectorSize, qint64 sectors, const QString& iconName = QString());

public:
    /**
     * @return the physical sector size the Device uses or -1 if unknown
     */
    qint64 physicalSectorSize() const;

    /**
     * @return the logical sector size the Device uses
     */
    qint64 logicalSectorSize() const;

    /**
     * @return the total number of sectors on the device
     */
    qint64 totalSectors() const;
};

#endif
