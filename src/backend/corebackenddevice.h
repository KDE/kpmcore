/*************************************************************************
 *  Copyright (C) 2010 by Volker Lanz <vl@fidra.de>                      *
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

#if !defined(KPMCORE_COREBACKENDDEVICE_H)

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
    CoreBackendDevice(const QString& deviceNode);
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
