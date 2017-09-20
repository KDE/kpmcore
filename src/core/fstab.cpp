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
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#include <QTextStream>

static void parseFsSpec(const QString& m_fsSpec, FstabEntryType& m_entryType, QString& m_deviceNode);
static QString findBlkIdDevice(const QString& token, const QString& value);

FstabEntry::FstabEntry(const QString& fsSpec, const QString& mountPoint, const QString& type, const QString& options, int dumpFreq, int passNumber, const QString& comment)
    : m_fsSpec(fsSpec)
    , m_mountPoint(mountPoint)
    , m_type(type)
    , m_dumpFreq(dumpFreq)
    , m_passNumber(passNumber)
    , m_comment(comment)
{
    m_options = options.split(QLatin1Char(','));
    parseFsSpec(m_fsSpec, m_entryType, m_deviceNode);
}

/**
  @param s the new value for the fs_spec field of fstab entry
*/
void FstabEntry::setFsSpec(const QString& s)
{
    m_fsSpec = s;
    parseFsSpec(m_fsSpec, m_entryType, m_deviceNode);
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
            QString line = rawLine.trimmed();
            if ( line.startsWith( QLatin1Char('#') ) || line.isEmpty()) {
                fstabEntries.append( { {}, {}, {}, {}, {}, {}, line } );
                continue;
            }

            QString comment = line.section( QLatin1Char('#'), 1 );
            QStringList splitLine = line.section( QLatin1Char('#'), 0, 0 ).split( QRegularExpression(QStringLiteral("[\\s]+")), QString::SkipEmptyParts );

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
                    fstabEntries.append( {splitLine.at(0), splitLine.at(1), splitLine.at(2), splitLine.at(3), splitLine.at(4).toInt(), splitLine.at(5).toInt(), comment.isEmpty() ? QString() : QLatin1Char('#') + comment } );
                    break;
                default:
                    fstabEntries.append( { {}, {}, {}, {}, {}, {}, QLatin1Char('#') + line } );
            }
        }

        fstabFile.close();
        if (fstabEntries.last().entryType() == comment && fstabEntries.last().comment().isEmpty())
            fstabEntries.removeLast();
    }

    return fstabEntries;
}

QStringList possibleMountPoints(const QString& deviceNode, const QString& fstabPath)
{
    QStringList mountPoints;
    QString canonicalPath = QFileInfo(deviceNode).canonicalFilePath();
    const FstabEntryList fstabEntryList = readFstabEntries( fstabPath );
    for (const FstabEntry &entry : fstabEntryList)
        if (QFileInfo(entry.deviceNode()).canonicalFilePath() == canonicalPath)
            mountPoints.append(entry.mountPoint());

    return mountPoints;
}

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

static void parseFsSpec(const QString& m_fsSpec, FstabEntryType& m_entryType, QString& m_deviceNode)
{
    m_entryType = FstabEntryType::comment;
    if (m_fsSpec.startsWith(QStringLiteral("UUID="))) {
        m_entryType = FstabEntryType::uuid;
        m_deviceNode = findBlkIdDevice(QStringLiteral("UUID"), QString(m_fsSpec).remove(QStringLiteral("UUID=")));
    } else if (m_fsSpec.startsWith(QStringLiteral("LABEL="))) {
        m_entryType = FstabEntryType::label;
        m_deviceNode = findBlkIdDevice(QStringLiteral("LABEL"), QString(m_fsSpec).remove(QStringLiteral("LABEL=")));
    } else if (m_fsSpec.startsWith(QStringLiteral("PARTUUID="))) {
        m_entryType = FstabEntryType::uuid;
        m_deviceNode = findBlkIdDevice(QStringLiteral("PARTUUID"), QString(m_fsSpec).remove(QStringLiteral("PARTUUID=")));
    } else if (m_fsSpec.startsWith(QStringLiteral("PARTLABEL="))) {
        m_entryType = FstabEntryType::label;
        m_deviceNode = findBlkIdDevice(QStringLiteral("PARTLABEL"), QString(m_fsSpec).remove(QStringLiteral("PARTLABEL=")));
    } else if (m_fsSpec.startsWith(QStringLiteral("/"))) {
        m_entryType = FstabEntryType::deviceNode;
        m_deviceNode = m_fsSpec;
    }
}

static void writeEntry(QFile& output, const FstabEntry& entry)
{
    QTextStream s(&output);
    if (entry.entryType() == FstabEntryType::comment) {
        s << entry.comment() << "\n";
        return;
    }

    QString options;
    if (entry.options().size() > 0) {
        options = entry.options().join(QLatin1Char(','));
        if (options.isEmpty())
            options = QStringLiteral("defaults");
    }
    else
        options = QStringLiteral("defaults");

    s << entry.fsSpec() << "\t"
      << (entry.mountPoint().isEmpty() ? QStringLiteral("none") : entry.mountPoint()) << "\t"
      << entry.type() << "\t"
      << options << "\t"
      << entry.dumpFreq() << "\t"
      << entry.passNumber() << "\t"
      << entry.comment() << "\n";
}

bool writeMountpoints(const FstabEntryList fstabEntries, const QString& filename)
{
    bool rval = true;
    const QString newFilename = QStringLiteral("%1.new").arg(filename);
    QFile out(newFilename);

    if (!out.open(QFile::ReadWrite | QFile::Truncate)) {
        qWarning() << "could not open output file " << newFilename;
        rval = false;
    } else {
        for (const auto &e : fstabEntries)
            writeEntry(out, e);

        out.close();

        const QString bakFilename = QStringLiteral("%1.bak").arg(filename);
        QFile::remove(bakFilename);

        if (QFile::exists(filename) && !QFile::rename(filename, bakFilename)) {
            qWarning() << "could not rename " << filename << " to " << bakFilename;
            rval = false;
        }

        if (rval && !QFile::rename(newFilename, filename)) {
            qWarning() << "could not rename " << newFilename << " to " << filename;
            rval = false;
        }
    }

    return rval;
}
