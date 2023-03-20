/*
    SPDX-FileCopyrightText: 2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2014-2018 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2015 Teo Mrnjavac <teo@kde.org>
    SPDX-FileCopyrightText: 2015 Chris Campbell <c.j.campbell@ed.ac.uk>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KPMCORE_COREBACKENDMANAGER_H
#define KPMCORE_COREBACKENDMANAGER_H

#include "util/libpartitionmanagerexport.h"

#include <memory>

#include <QStringList>
#include <QVector>

class QString;
class KPluginMetaData;
class CoreBackend;
struct CoreBackendManagerPrivate;

/**
  * The backend manager class.
  *
  * This is basically a singleton class to give the application access to the currently
  * selected backend and also to manage the available backend plugins.
  * @author Volker Lanz <vl@fidra.de>
  */
class LIBKPMCORE_EXPORT CoreBackendManager
{
    CoreBackendManager();
    ~CoreBackendManager();

public:
    /**
      * @return pointer to ourselves
      */
    static CoreBackendManager* self();

    /**
      * @return the name of the default backend plugin
      */
    static QString defaultBackendName() {
        return QStringLiteral("pmsfdiskbackendplugin");
    }

    /**
      * @return a list of available backend plugins
      */
    QVector<KPluginMetaData> list() const;

    /**
       * Loads the given backend plugin into the application.
       * @param name the name of the plugin to load
       * @return true on success
       */
    bool load(const QString& name);

    /**
      * Unload the current plugin.
      */
    void unload();

    /**
      * @return a pointer to the currently loaded backend
      */
    CoreBackend* backend();

private:
    std::unique_ptr<CoreBackendManagerPrivate> d;
};

#endif
