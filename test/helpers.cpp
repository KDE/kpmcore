#include "helpers.h"

#include "backend/corebackendmanager.h"

#include <QDebug>
#include <QString>

static bool s_KPMcoreInited = false;

KPMCoreInitializer::KPMCoreInitializer() :
    m_isValid( s_KPMcoreInited )
{
    if ( !s_KPMcoreInited )
    {
        QByteArray env = qgetenv( "KPMCORE_BACKEND" );
        auto backendName = env.isEmpty() ? CoreBackendManager::defaultBackendName() : QString::fromLatin1(env);

        if ( !CoreBackendManager::self()->load( backendName ) )
        {
            qWarning() << "Failed to load backend plugin" << backendName;
        }
        else
        {
            m_isValid = s_KPMcoreInited = true;
        }
    }
}

KPMCoreInitializer::KPMCoreInitializer(const QString& backendName) :
    m_isValid( s_KPMcoreInited )
{
    if (!s_KPMcoreInited)
    {
        if ( !CoreBackendManager::self()->load( backendName ) )
        {
            qWarning() << "Failed to load backend plugin" << backendName;
        }
        else
        {
            m_isValid = s_KPMcoreInited = true;
        }
    }
}

KPMCoreInitializer::KPMCoreInitializer(const char *backend) : KPMCoreInitializer( QString::fromLatin1(backend) )
{
}
