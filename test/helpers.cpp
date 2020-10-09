/*
    SPDX-FileCopyrightText: 2017 by Adriaan de Groot <groot@kde.org>
    SPDX-FileCopyrightText: 2018 Andrius Štikonas <andrius@stikonas.eu>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "helpers.h"

#include "backend/corebackendmanager.h"
#include "util/externalcommand.h"

#include <QDebug>
#include <QString>

static bool s_initialized = false;

KPMCoreInitializer::KPMCoreInitializer() :
    m_isValid( s_initialized )
{
    if ( !s_initialized )
    {
        QByteArray env = qgetenv( "KPMCORE_BACKEND" );
        auto backendName = env.isEmpty() ? CoreBackendManager::defaultBackendName() : QString::fromLatin1( env );

        if ( !CoreBackendManager::self()->load( backendName ) )
            qWarning() << "Failed to load backend plugin" << backendName;
        else
            m_isValid = s_initialized = true;
    }
}

KPMCoreInitializer::KPMCoreInitializer( const QString& backendName ) :
    m_isValid( s_initialized )
{
    if ( !s_initialized )
    {
        if ( !CoreBackendManager::self()->load( backendName ) )
            qWarning() << "Failed to load backend plugin" << backendName;
        else
            m_isValid = s_initialized = true;
    }
}

KPMCoreInitializer::KPMCoreInitializer( const char* backend ) : KPMCoreInitializer( QString::fromLatin1( backend ) )
{
}

