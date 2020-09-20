/*
    SPDX-FileCopyrightText: 2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2014-2016 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2015 Chris Campbell <c.j.campbell@ed.ac.uk>
    SPDX-FileCopyrightText: 2016 Chantara Tith <tith.chantara@gmail.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KPMCORE_DEVICESCANNER_H
#define KPMCORE_DEVICESCANNER_H

#include "util/libpartitionmanagerexport.h"

#include <QThread>

class OperationStack;

/** Thread to scan for all available Devices on this computer.

    This class is used to find all Devices on the computer and to create new Device instances for each of them. It's subclassing QThread to run asynchronously.

    @author Volker Lanz <vl@fidra.de>
*/
class LIBKPMCORE_EXPORT DeviceScanner : public QThread
{
    Q_OBJECT

public:
    DeviceScanner(QObject* parent, OperationStack& ostack);

public:
    void clear(); /**< clear Devices and the OperationStack */
    void scan(); /**< do the actual scanning; blocks if called directly */
    void setupConnections();

Q_SIGNALS:
    void progress(const QString& deviceNode, int progress);

protected:
    void run() override;
    OperationStack& operationStack() {
        return m_OperationStack;
    }
    const OperationStack& operationStack() const {
        return m_OperationStack;
    }

private:
    OperationStack& m_OperationStack;
};

#endif
