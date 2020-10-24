/*
    SPDX-FileCopyrightText: 2008-2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2008 Laurent Montel <montel@kde.org>
    SPDX-FileCopyrightText: 2014-2016 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2015 Teo Mrnjavac <teo@kde.org>
    SPDX-FileCopyrightText: 2015 Chris Campbell <c.j.campbell@ed.ac.uk>
    SPDX-FileCopyrightText: 2019 Yuri Chornoivan <yurchor@ukr.net>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KPMCORE_OPERATIONSTACK_H
#define KPMCORE_OPERATIONSTACK_H

#include "util/libpartitionmanagerexport.h"

#include <QObject>
#include <QList>
#include <QReadWriteLock>

#include <QtGlobal>

class Device;
class Partition;
class Operation;
class DeviceScanner;

/** The list of Operations the user wants to have performed.

    OperationStack also handles the Devices that were found on this computer and the merging of
    Operations, e.g., when the user first creates a Partition, then deletes it.

    @author Volker Lanz <vl@fidra.de>
*/
class LIBKPMCORE_EXPORT OperationStack : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(OperationStack)

    friend class DeviceScanner;

public:
    typedef QList<Device*> Devices;
    typedef QList<Operation*> Operations;

public:
    explicit OperationStack(QObject* parent = nullptr);
    ~OperationStack() override;

Q_SIGNALS:
    void operationsChanged();
    void devicesChanged();

public:
    void push(Operation* o);
    void pop();
    bool contains(const Partition* p) const;
    void clearOperations();
    int size() const {
        return operations().size();    /**< @return number of operations */
    }

    Devices& previewDevices() {
        return m_PreviewDevices;    /**< @return the list of Devices */
    }
    const Devices& previewDevices() const {
        return m_PreviewDevices;    /**< @return the list of Devices */
    }

    Operations& operations() {
        return m_Operations;    /**< @return the list of operations */
    }
    const Operations& operations() const {
        return m_Operations;    /**< @return the list of operations */
    }

    Device* findDeviceForPartition(const Partition* p);

    QReadWriteLock& lock() {
        return m_Lock;
    }

protected:
    void clearDevices();
    void addDevice(Device* d);
    void sortDevices();

    bool mergeNewOperation(Operation*& currentOp, Operation*& pushedOp);
    bool mergeCopyOperation(Operation*& currentOp, Operation*& pushedOp);
    bool mergeRestoreOperation(Operation*& currentOp, Operation*& pushedOp);
    bool mergePartFlagsOperation(Operation*& currentOp, Operation*& pushedOp);
    bool mergePartLabelOperation(Operation*& currentOp, Operation*& pushedOp);
    bool mergeCreatePartitionTableOperation(Operation*& currentOp, Operation*& pushedOp);
    bool mergeResizeVolumeGroupResizeOperation(Operation*& pushedOp);

private:
    Operations m_Operations;
    mutable Devices m_PreviewDevices;
    QReadWriteLock m_Lock;
};

#endif
