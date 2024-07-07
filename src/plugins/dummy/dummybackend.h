/*
    SPDX-FileCopyrightText: 2008-2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2015 Teo Mrnjavac <teo@kde.org>
    SPDX-FileCopyrightText: 2016-2019 Andrius Štikonas <andrius@stikonas.eu>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KPMCORE_DUMMYBACKEND_H
#define KPMCORE_DUMMYBACKEND_H

#include "backend/corebackend.h"

#include <QList>
#include <QVariant>

class Device;
class KPluginFactory;
class QString;

/** Dummy backend plugin that doesn't really do anything.

    @author Volker Lanz <vl@fidra.de>
*/
class DummyBackend : public CoreBackend
{
    Q_DISABLE_COPY(DummyBackend)

public:
    DummyBackend(QObject* parent, const QList<QVariant>& args);

public:
    void initFSSupport() override;

    QList<Device*> scanDevices(const ScanFlags scanFlags = {}) override;
    std::unique_ptr<CoreBackendDevice> openDevice(const Device& d) override;
    std::unique_ptr<CoreBackendDevice> openDeviceExclusive(const Device& d) override;
    bool closeDevice(std::unique_ptr<CoreBackendDevice> coreDevice) override;
    Device* scanDevice(const QString& deviceNode) override;
    FileSystem::Type detectFileSystem(const QString& deviceNode) override;
    QString readLabel(const QString& deviceNode) const override;
    QString readUUID(const QString& deviceNode) const override;
};

#endif
