/*************************************************************************
 *  Copyright (C) 2010 by Volker Lanz <vl@fidra.de>                      *
 *  Copyright (C) 2015 by Teo Mrnjavac <teo@kde.org>                     *
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

#include "backend/corebackendmanager.h"
#include "backend/corebackend.h"

#include <QDebug>
#include <QStringList>
#include <QString>
#include <QVector>

#include <KLocalizedString>
#include <KPluginFactory>
#include <KPluginLoader>
#include <KPluginMetaData>

CoreBackendManager::CoreBackendManager() :
    m_Backend(nullptr)
{
}

CoreBackendManager* CoreBackendManager::self()
{
    static CoreBackendManager* instance = nullptr;

    if (instance == nullptr)
        instance = new CoreBackendManager;

    return instance;
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
        m_Backend = factory->create<CoreBackend>(nullptr);

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
    delete m_Backend;
    m_Backend = nullptr;
}
