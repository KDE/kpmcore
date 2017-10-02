# KPMcore

> KPMcore, the KDE Partition Manager core, is a library for examining
> and modifying partitions, disk devices, and filesystems on a
> Linux system. It provides a unified programming interface over
> top of (external) system-manipulation tools.

KPMcore is a library for examining and manipulating all facets
of storage devices on a system:
* raw disk devices
* partition tables on a device
* filesystems within a partition

There are multiple backends so that KPMcore can support different
operating systems, although the only functional backend is the
one for Linux systems:
* libparted backend (Linux)
* null backend

## Using KPMcore

Most of the usage information on KPMcore is included in the API
documentation; this section contains only high-level usage information.

### Finding KPMcore with CMake

KPMcore supports CMake as (meta-)build system and installs suitable
CMake support files. Typical use of of KPMcore in a `CMakeLists.txt`
looks like this:

```
    find_package( KPMcore 3.2 REQUIRED )
    include_directories( ${KPMCORE_INCLUDE_DIR} )
    target_link_libraries( target kpmcore )
```

There are no imported targets defined for KPMcore.

### Initialization

An application must initialize the library and load a suitable
backend before using KPMcore functions. By convention, the
environment variable `KPMCORE_BACKEND` names a backend,
and typical initialization code will look like this (or use the
class `KPMCoreInitializer` from `test/helpers.h`):

```
    #include <kpmcore/backend/corebackendmanager.h>
    #include <QDebug>

    bool initKPMcore()
    {
        static bool inited = false;
        if ( inited ) return true;

        QByteArray env = qgetenv( "KPMCORE_BACKEND" );
        auto backendName = env.isEmpty() ? CoreBackendManager::defaultBackendName() : env;
        if ( !CoreBackendManager::self()->load( backendName )
        {
            qWarning() << "Failed to load backend plugin" << backendName;
            return false;
        }
        inited = true;
        return true;
    }
```

This code uses the environment variable if set, and otherwise falls
back to a default backend suitable for the current platform.

Calling KPMcore functions before the library is initialized will
result in undefined behavior.
