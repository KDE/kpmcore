/*************************************************************************
 *  Copyright (C) 2009, 2010 by Volker Lanz <vl@fidra.de>                *
 *  Copyright (C) 2016 by Teo Mrnjavac <teo@kde.org>                     *
 *  Copyright (C) 2017 by Andrius Štikonas <andrius@stikonas.eu>         *
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

#include "core/fstab.h"

#include <blkid/blkid.h>

#include <QChar>
#include <QFile>
#include <QFileInfo>

static QString findBlkIdDevice(const QString& token, const QString& value)
{
    blkid_cache cache;
    QString rval;

    if (blkid_get_cache(&cache, nullptr) == 0) {
        if (char* c = blkid_evaluate_tag(token.toLocal8Bit().constData(), value.toLocal8Bit().constData(), &cache)) {
            rval = QString::fromLocal8Bit(c);
            free(c);
        }

        blkid_put_cache(cache);
    }

    return rval;
}

FstabEntry::FstabEntry(const QString& fsSpec, const QString& mountPoint, const QString& type, const QString& options, int dumpFreq, int passNumber, const QString& comment)
    : m_fsSpec(fsSpec)
    , m_mountPoint(mountPoint)
    , m_type(type)
    , m_options(options)
    , m_dumpFreq(dumpFreq)
    , m_passNumber(passNumber)
    , m_comment(comment)
{
    m_entryType = FstabEntryType::comment;
    if (fsSpec.startsWith(QStringLiteral("UUID="))) {
        m_entryType = FstabEntryType::uuid;
        m_deviceNode = findBlkIdDevice(QStringLiteral("UUID"), QString(fsSpec).remove(QStringLiteral("UUID=")));
    } else if (fsSpec.startsWith(QStringLiteral("LABEL="))) {
        m_entryType = FstabEntryType::label;
        m_deviceNode = findBlkIdDevice(QStringLiteral("LABEL"), QString(fsSpec).remove(QStringLiteral("LABEL=")));
    } else if (fsSpec.startsWith(QStringLiteral("PARTUUID="))) {
        m_entryType = FstabEntryType::uuid;
        m_deviceNode = findBlkIdDevice(QStringLiteral("PARTUUID"), QString(fsSpec).remove(QStringLiteral("PARTUUID=")));
    } else if (fsSpec.startsWith(QStringLiteral("PARTLABEL="))) {
        m_entryType = FstabEntryType::label;
        m_deviceNode = findBlkIdDevice(QStringLiteral("PARTLABEL"), QString(fsSpec).remove(QStringLiteral("PARTLABEL=")));
    } else if (fsSpec.startsWith(QStringLiteral("/"))) {
        m_entryType = FstabEntryType::deviceNode;
        m_deviceNode = fsSpec;
    }
}

FstabEntryList readFstabEntries( const QString& fstabPath )
{
    FstabEntryList fstabEntries;
    QFile fstabFile( fstabPath );
    if ( fstabFile.open( QIODevice::ReadOnly | QIODevice::Text ) )
    {
        const QStringList fstabLines = QString::fromLocal8Bit(fstabFile.readAll()).split( QLatin1Char('\n') );
        for ( const QString& rawLine : fstabLines )
        {
            QString line = rawLine.simplified();
            if ( line.startsWith( QLatin1Char('#') ) || line.isEmpty()) {
                fstabEntries.append( { {}, {}, {}, {}, {}, {}, line } );
                continue;
            }

            QString comment = line.section( QLatin1Char('#'), 1 );
            QStringList splitLine = line.section( QLatin1Char('#'), 0, 0 ).split( QLatin1Char(' ') );

            // We now split the standard components of /etc/fstab entry:
            // (0) path, or UUID, or LABEL, etc,
            // (1) mount point,
            // (2) file system type,
            // (3) options,
            // (4) dump frequency (optional, defaults to 0), no comment is allowed if omitted,
            // (5) pass number (optional, defaults to 0), no comment is allowed if omitted,
            // (#) comment (optional).
            switch (splitLine.length()) {
                case 4:
                    fstabEntries.append( {splitLine.at(0), splitLine.at(1), splitLine.at(2), splitLine.at(3) } );
                    break;
                case 5:
                    fstabEntries.append( {splitLine.at(0), splitLine.at(1), splitLine.at(2), splitLine.at(3), splitLine.at(4).toInt() } );
                    break;
                case 6:
                    fstabEntries.append( {splitLine.at(0), splitLine.at(1), splitLine.at(2), splitLine.at(3), splitLine.at(4).toInt(), splitLine.at(5).toInt(), comment } );
                    break;
                default:
                    fstabEntries.append( { {}, {}, {}, {}, {}, {}, QLatin1Char('#') + line } );
            }
        }

        fstabFile.close();
    }

    return fstabEntries;
}

QStringList possibleMountPoints(const QString& deviceNode, const QString& fstabPath)
{
    QStringList mountPoints;
    QFileInfo kernelPath(deviceNode);
    QString canonicalPath = kernelPath.canonicalFilePath();
    const FstabEntryList fstabEntryList = readFstabEntries( fstabPath );
    for (const FstabEntry &entry : fstabEntryList) {
        QFileInfo kernelPath2(entry.deviceNode());
        if (kernelPath2.canonicalFilePath() == canonicalPath)
            mountPoints.append(entry.mountPoint());
    }
    return mountPoints;
}
