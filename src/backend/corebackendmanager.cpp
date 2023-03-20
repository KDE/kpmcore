/*
    SPDX-FileCopyrightText: 2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2014-2021 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2015 Teo Mrnjavac <teo@kde.org>
    SPDX-FileCopyrightText: 2018 Caio Jordão Carvalho <caiojcarvalho@gmail.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "backend/corebackendmanager.h"
#include "backend/corebackend.h"

#include <QCoreApplication>
#include <QDebug>
#include <QString>
#include <QVector>

#include <KLocalizedString>
#include <KPluginFactory>
#include <KPluginMetaData>

struct CoreBackendManagerPrivate
{
    CoreBackend *m_Backend;
};

CoreBackendManager::CoreBackendManager() :
    d(std::make_unique<CoreBackendManagerPrivate>())
{
}

CoreBackendManager::~CoreBackendManager()
{
}

CoreBackendManager* CoreBackendManager::self()
{
    static CoreBackendManager* instance = nullptr;

    if (instance == nullptr)
        instance = new CoreBackendManager;

    return instance;
}

CoreBackend* CoreBackendManager::backend()
{
    return d->m_Backend;
}

QVector<KPluginMetaData> CoreBackendManager::list() const
{
    return KPluginMetaData::findPlugins(QStringLiteral("kpmcore"));
}

bool CoreBackendManager::load(const QString& name)
{
    if (backend())
        unload();

    QString path = QStringLiteral("kpmcore/") + name;

    KPluginMetaData metadata(path);
    d->m_Backend = KPluginFactory::instantiatePlugin<CoreBackend>(metadata).plugin;

    if (!backend()) {
        qWarning() << "Could not create instance of plugin  " << name;
        return false;
    }

    QString id = metadata.pluginId();
    QString version = metadata.version();
    if (id.isEmpty())
        return false;

    backend()->setId(id);
    backend()->setVersion(version);
    qDebug() << "Loaded backend plugin: " << backend()->id();
    return true;
}

void CoreBackendManager::unload()
{
}
