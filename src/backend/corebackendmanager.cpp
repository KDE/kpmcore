/*************************************************************************
 *  Copyright (C) 2010 by Volker Lanz <vl@fidra.de>                      *
 *  Copyright (C) 2015 by Teo Mrnjavac <teo@kde.org>                     *
 *  Copyright (C) 2016-2018 by Andrius Štikonas <andrius@stikonas.eu>    *
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

#include <QCoreApplication>
#include <QDebug>
#include <QDBusInterface>
#include <QStringList>
#include <QString>
#include <QVector>
#include <QUuid>

#include <KAuth>
#include <KLocalizedString>
#include <KPluginFactory>
#include <KPluginLoader>
#include <KPluginMetaData>

struct CoreBackendManagerPrivate
{
    KAuth::ExecuteJob *m_job;
    CoreBackend *m_Backend;

    QString m_Uuid;
};

CoreBackendManager::CoreBackendManager() :
    d(std::make_unique<CoreBackendManagerPrivate>())
{
    startExternalCommandHelper();
}

CoreBackendManager* CoreBackendManager::self()
{
    static CoreBackendManager* instance = nullptr;

    if (instance == nullptr)
        instance = new CoreBackendManager;

    return instance;
}

CoreBackend* CoreBackendManager::backend() {
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

void CoreBackendManager::startExternalCommandHelper()
{
    KAuth::Action action = KAuth::Action(QStringLiteral("org.kde.kpmcore.externalcommand.init"));
    action.setHelperId(QStringLiteral("org.kde.kpmcore.externalcommand"));
    action.setTimeout(10 * 24 * 3600 * 1000); // 10 days
    QVariantMap arguments;
    d->m_Uuid = QUuid::createUuid().toString();
    arguments.insert(QStringLiteral("callerUuid"), Uuid());
    action.setArguments(arguments);
    d->m_job = action.execute();
    job()->start();

    // Wait until ExternalCommand Helper is ready (helper sends newData signal just before it enters event loop)
    QEventLoop loop;
    auto exitLoop = [&] () { loop.exit(); };
    auto conn = QObject::connect(job(), &KAuth::ExecuteJob::newData, exitLoop);
    QObject::connect(job(), &KJob::finished, [=] () { if(d->m_job->error()) exitLoop(); } );
    loop.exec();
    QObject::disconnect(conn);
}

void CoreBackendManager::stopExternalCommandHelper()
{
    QDBusInterface iface(QStringLiteral("org.kde.kpmcore.helperinterface"), QStringLiteral("/Helper"), QStringLiteral("org.kde.kpmcore.externalcommand"), QDBusConnection::systemBus());
    if (iface.isValid())
        iface.call(QStringLiteral("exit"), CoreBackendManager::self()->Uuid());
}

KAuth::ExecuteJob* CoreBackendManager::job() {
    return d->m_job;
}

QString& CoreBackendManager::Uuid() {
    return d->m_Uuid;
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
