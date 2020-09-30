/*
    SPDX-FileCopyrightText: 2016 Chantara Tith <tith.chantara@gmail.com>
    SPDX-FileCopyrightText: 2016-2018 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2019 Caio Jordão Carvalho <caiojcarvalho@gmail.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KPMCORE_VOLUMEMANAGERDEVICE_H
#define KPMCORE_VOLUMEMANAGERDEVICE_H

#include "util/libpartitionmanagerexport.h"
#include "core/device.h"

#include <QString>
#include <QStringList>
#include <QObject>
#include <QtGlobal>

class VolumeManagerDevicePrivate;

/** A Volume Manager of physical devices represented as an abstract device.
 *
 *   VolumeManagerDevice is an abstract device class for volume manager. e.g: LVM, SoftRAID.
 *   example of physical device: /dev/sda, /dev/sdb1.
 *
 *   Devices are the outermost entity; they contain a PartitionTable that itself contains Partitions.
 *
 *   @see Device, PartitionTable, Partition
 */
class LIBKPMCORE_EXPORT VolumeManagerDevice : public Device
{
    Q_DISABLE_COPY(VolumeManagerDevice)

public:
    VolumeManagerDevice(std::shared_ptr<VolumeManagerDevicePrivate> d, const QString& name, const QString& deviceNode, const qint64 logicalSectorSize, const qint64 totalLogical, const QString& iconName = QString(), Device::Type type = Device::Type::Unknown_Device);

    /**
     *  @return list of physical device's path that makes up volumeManagerDevice.(e.g: /dev/sda, /dev/sdb1)
     */
    virtual const QStringList deviceNodes() const = 0;

    /**
     * @return list of logical partition's path.
     */
    virtual const QStringList& partitionNodes() const = 0;

    /**
     * @return size of logical partition at the given path in bytes.
     */
    virtual qint64 partitionSize(QString& partitionPath) const = 0;

protected:

    /** Initialize device's partition table and partitions.
     *
     */
    virtual void initPartitions() = 0;

    /** absolute sector as represented inside the device's partitionTable
     *
     *  For VolumeMangerDevice to works with the rest of the codebase, partitions are stringed
     *  one after another to create a representation of PartitionTable and partition just like
     *  real disk device.
     *
     *  @param partitionPath logical partition path
     *  @sector sector value to be mapped (if 0, will return start sector of the partition)
     *  @return absolute sector value as represented inside device's partitionTable
     */
    virtual qint64 mappedSector(const QString& partitionPath, qint64 sector) const = 0;

public:

    static void scanDevices(QList<Device*>& devices);

    /** join deviceNodes together into comma-separated list
     *
     *  @return comma-separated list of deviceNodes
     */
    virtual QString prettyDeviceNodeList() const;

    /** Resize device total number of logical sectors.
     *
     * @param n Number of sectors.
     */
    void setTotalLogical(qint64 n);
};

#endif
