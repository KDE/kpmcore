/*************************************************************************
 *  Copyright 2017 by Adriaan de Groot <groot@kde.org>                   *
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

//  SPDX-License-Identifier: GPL-3.0+

#include "helpers.h"

#include "backend/corebackendmanager.h"

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
