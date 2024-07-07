/*
    SPDX-FileCopyrightText: 2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2014-2020 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2015 Teo Mrnjavac <teo@kde.org>
    SPDX-FileCopyrightText: 2015 Chris Campbell <c.j.campbell@ed.ac.uk>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KPMCORE_COREBACKEND_H
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

enum class ScanFlag : uint8_t {
    includeReadOnly = 0x1, /**< devices that are read-only according to the kernel */
    includeLoopback = 0x2,
};
Q_DECLARE_FLAGS(ScanFlags, ScanFlag)
Q_DECLARE_OPERATORS_FOR_FLAGS(ScanFlags)

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
    ~CoreBackend() override;

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
      * @param scanFlags can be used to expand the list of scanned devices.
      * @return a QList of pointers to Device instances. The caller is responsible
      *         for deleting these objects.
      * @note A Device object is a description of the device, not
      *         an object to operate on. See openDevice().
      */
    virtual QList<Device*> scanDevices(const ScanFlags scanFlags) = 0;

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

    static bool isPolkitInstalledCorrectly();

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
