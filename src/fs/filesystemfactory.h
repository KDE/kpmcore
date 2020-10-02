/*
    SPDX-FileCopyrightText: 2008-2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2015 Chris Campbell <c.j.campbell@ed.ac.uk>
    SPDX-FileCopyrightText: 2016-2020 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2020 Arnaud Ferraris <arnaud.ferraris@collabora.com>
    SPDX-FileCopyrightText: 2020 Gaël PORTAY <gael.portay@collabora.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KPMCORE_FILESYSTEMFACTORY_H
#define KPMCORE_FILESYSTEMFACTORY_H

#include "fs/filesystem.h"

#include "util/libpartitionmanagerexport.h"

#include <QMap>
#include <QtGlobal>

class QString;

/** Factory to create instances of FileSystem.
    @author Volker Lanz <vl@fidra.de>
 */
class LIBKPMCORE_EXPORT FileSystemFactory
{
public:
    /** map of FileSystem::Types to pointers of FileSystem */
    typedef QMap<FileSystem::Type, FileSystem*> FileSystems;

private:
    FileSystemFactory();

public:
    static void init();
    static FileSystem* create(FileSystem::Type t, qint64 firstsector, qint64 lastsector, qint64 sectorSize, qint64 sectorsused = -1, const QString& label = QString(), const QVariantMap& features = {}, const QString& uuid = QString());
    static FileSystem* create(const FileSystem& other);
    static FileSystem* cloneWithNewType(FileSystem::Type newType, const FileSystem& other);
    static const FileSystems& map();

private:
    static FileSystems m_FileSystems;
};

#endif
