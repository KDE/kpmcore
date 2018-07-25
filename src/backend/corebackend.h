/*************************************************************************
 *  Copyright (C) 2010 by Volker Lanz <vl@fidra.de>                      *
 *  Copyright (C) 2016 by Andrius Štikonas <andrius@stikonas.eu>         *
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

#if !defined(KPMCORE_COREBACKEND_H)

#define KPMCORE_COREBACKEND_H

#include "util/libpartitionmanagerexport.h"
#include "fs/filesystem.h"

#include <memory>

#include <QObject>
#include <QList>

class CoreBackendManager;
class CoreBackendDevice;
struct CoreBackendPrivate;
class Device;
class PartitionTable;

class QString;

/**
  * Interface class for backend plugins.
  * @author Volker Lanz <vl@fidra.de>
  */

class LIBKPMCORE_EXPORT CoreBackend : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(CoreBackend)

    friend class CoreBackendManager;

protected:
    CoreBackend();
    virtual ~CoreBackend();

Q_SIGNALS:
    /**
     * Emitted to inform about progress of any kind.
      * @param i the progress in percent (from 0 to 100)
     */
    void progress(int i);

    /**
      * Emitted to inform about scan progress.
      * @param deviceNode the device being scanned just now (e.g. "/dev/sda")
      * @param i the progress in percent (from 0 to 100)
      */
    void scanProgress(const QString& deviceNode, int i);

public:
    /**
      * Return the plugin's unique Id from JSON metadata
      * @return the plugin's unique Id from JSON metadata
      */
    QString id();

    /**
      * Return the plugin's version from JSON metadata
      * @return the plugin's version from JSON metadata
      */
    QString version();

    /**
      * Initialize the plugin's FileSystem support
      */
    virtual void initFSSupport() = 0;

    /**
      * Scan for devices in the system.
      * @param excludeReadOnly when true, devices that are read-only
      *         according to the kernel are left out of the list.
      *         When false (the default) all devices, writable or
      *         not, including CD ROM devices, are returned.
      * @return a QList of pointers to Device instances. The caller is responsible
      *         for deleting these objects.
      * @note A Device object is a description of the device, not
      *         an object to operate on. See openDevice().
      */
    virtual QList<Device*> scanDevices(bool excludeReadOnly = false) = 0;

    /**
      * Scan a single device in the system.
      * @param deviceNode The path to the device that is to be scanned (e.g. /dev/sda1)
      * @return FileSystem type of the device on deviceNode
      */
    virtual FileSystem::Type detectFileSystem(const QString& deviceNode) = 0;

    /**
      * Read a file system label
      * @param deviceNode The path to the device that is to be scanned (e.g. /dev/sda1)
      * @return FileSystem label on deviceNode
      */
    virtual QString readLabel(const QString& deviceNode) const = 0;

    /**
      * Read a file system UUID
      * @param deviceNode The path to the device that is to be scanned (e.g. /dev/sda1)
      * @return FileSystem UUID on deviceNode
      */
    virtual QString readUUID(const QString& deviceNode) const = 0;

    /**
      * Scan a single device in the system.
      * @param deviceNode The path to the device that is to be scanned (e.g. /dev/sda)
      * @return a pointer to a Device instance. The caller is responsible for deleting
      *         this object.
      */
    virtual Device* scanDevice(const QString& deviceNode) = 0;

    /**
      * Open a device for reading.
      * @param deviceNode The path of the device that is to be opened (e.g. /dev/sda)
      * @return a pointer to a CoreBackendDevice or nullptr if the open failed.
      */
    virtual std::unique_ptr<CoreBackendDevice> openDevice(const Device& d) = 0;

    /**
      * Open a device in exclusive mode for writing.
      * @param deviceNode The path of the device that is to be opened (e.g. /dev/sda)
      * @return a pointer to a CoreBackendDevice or nullptr if the open failed.
      */
    virtual std::unique_ptr<CoreBackendDevice> openDeviceExclusive(const Device& d) = 0;

    /**
      * Close a CoreBackendDevice that has previously been opened.
      * @param core_device Pointer to the CoreBackendDevice to be closed. Must not be nullptr.
      * @return true if closing the CoreBackendDevice succeeded, otherwise false.
      */
    virtual bool closeDevice(std::unique_ptr<CoreBackendDevice> coreDevice) = 0;

    /**
      * Emit progress.
      * @param i the progress in percent (from 0 to 100)
      * This is used to emit a progress() signal from somewhere deep inside the plugin
      * backend code if that is ever necessary.
      */
    virtual void emitProgress(int i);

    /**
      * Emit scan progress.
      * @param deviceNode the path to the device just being scanned (e.g. /dev/sda)
      * @param i the progress in percent (from 0 to 100)
      * This is used to emit a scanProgress() signal from the backend device scanning
      * code.
      */
    virtual void emitScanProgress(const QString& deviceNode, int i);

protected:
    static void setPartitionTableForDevice(Device& d, PartitionTable* p);
    static void setPartitionTableMaxPrimaries(PartitionTable& p, qint32 max_primaries);

private:
    void setId(const QString& id);
    void setVersion(const QString& version);

private:
    std::unique_ptr<CoreBackendPrivate> d;
};

#endif
