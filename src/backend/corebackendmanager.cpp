/*
    SPDX-FileCopyrightText: 2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2014-2018 Andrius Štikonas <andrius@stikonas.eu>
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
#include <KPluginLoader>
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
    auto filter = [&](const KPluginMetaData &metaData) {
        return metaData.serviceTypes().contains(QStringLiteral("PartitionManager/Plugin")) &&
               metaData.category().contains(QStringLiteral("BackendPlugin"));
    };

    // find backend plugins in standard path (e.g. /usr/lib64/qt5/plugins) using filter from above
    return KPluginLoader::findPlugins(QString(), filter);
}

bool CoreBackendManager::load(const QString& name)
{
    if (backend())
        unload();

    KPluginLoader loader(name);

    KPluginFactory* factory = loader.factory();

    if (factory != nullptr) {
        d->m_Backend = factory->create<CoreBackend>(nullptr);

        QString id = loader.metaData().toVariantMap().value(QStringLiteral("MetaData"))
                     .toMap().value(QStringLiteral("KPlugin")).toMap().value(QStringLiteral("Id")).toString();
        QString version = loader.metaData().toVariantMap().value(QStringLiteral("MetaData"))
                          .toMap().value(QStringLiteral("KPlugin")).toMap().value(QStringLiteral("Version")).toString();
        if (id.isEmpty())
            return false;

        backend()->setId(id);
        backend()->setVersion(version);
        qDebug() << "Loaded backend plugin: " << backend()->id();

        return true;
    }

    qWarning() << "Could not load plugin for core backend " << name << ": " << loader.errorString();
    return false;
}

void CoreBackendManager::unload()
{
}
