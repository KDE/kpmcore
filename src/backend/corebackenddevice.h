/*
    SPDX-FileCopyrightText: 2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2014-2018 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2015 Teo Mrnjavac <teo@kde.org>
    SPDX-FileCopyrightText: 2015 Chris Campbell <c.j.campbell@ed.ac.uk>
    SPDX-FileCopyrightText: 2017 Adriaan de Groot <groot@kde.org>
    SPDX-FileCopyrightText: 2019 Yuri Chornoivan <yurchor@ukr.net

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KPMCORE_COREBACKENDDEVICE_H
#define KPMCORE_COREBACKENDDEVICE_H

#include <memory>
#include <QString>

class CoreBackendPartition;
class CoreBackendPartitionTable;
class Partition;
class PartitionTable;
class Report;

/**
  * Interface class for devices in the backend plugin.
  * For a device description, see Device. This
  * CoreBackendDevice can be used for (read- and) write
  * operations on the raw device.
  *
  * @author Volker Lanz <vl@fidra.de>
  */
class CoreBackendDevice
{
public:
    explicit CoreBackendDevice(const QString& deviceNode);
    virtual ~CoreBackendDevice() {}

public:
    /**
      * Get the device path for this device (e.g. "/dev/sda")
      * @return the device path
      */
    virtual const QString& deviceNode() const {
        return m_DeviceNode;
    }

    /**
      * Determine if this device is opened in exclusive mode.
      * @return true if it is opened in exclusive mode, otherwise false
      */
    virtual bool isExclusive() const {
        return m_Exclusive;
    }

    /**
      * Open the backend device
      * @return true if successful
      */
    virtual bool open() = 0;

    /**
      * Open the backend device in exclusive mode
      * @return true if successful
      */
    virtual bool openExclusive() = 0;

    /**
      * Close the backend device
      * @return true if successful
      */
    virtual bool close() = 0;

    /**
      * Open this backend device's partition table
      * @return a pointer to the CoreBackendPartitionTable for this device or nullptr in case
      *         of errors
      */
    virtual std::unique_ptr<CoreBackendPartitionTable> openPartitionTable() = 0;

    /**
      * Create a new partition table on this device.
      * @param report the Report to write information to
      * @param ptable the PartitionTable to create on this backend device
      * @return true if successful
      */
    virtual bool createPartitionTable(Report& report, const PartitionTable& ptable) = 0;

protected:
    void setExclusive(bool b) {
        m_Exclusive = b;
    }

private:
    const QString m_DeviceNode;
    bool m_Exclusive;
};

#endif
