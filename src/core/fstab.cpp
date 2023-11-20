/*
    SPDX-FileCopyrightText: 2017-2020 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2019 Adriaan de Groot <groot@kde.org>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "core/fstab.h"
#include "util/externalcommand.h"
#include "util/report.h"

#include <algorithm>
#include <array>

#if defined(Q_OS_LINUX)
    #include <blkid/blkid.h>
#endif

#include <QChar>
#include <QDebug>
#include <QFileInfo>
#include <QRegularExpression>
#include <QTemporaryFile>
#include <QTextStream>

using namespace Qt::StringLiterals;

static void parseFsSpec(const QString& m_fsSpec, FstabEntry::Type& m_entryType, QString& m_deviceNode);
static QString findBlkIdDevice(const char *token, const QString& value);
static void writeEntry(QTextStream& s, const FstabEntry& entry, std::array<unsigned int, 4> columnWidth);
std::array<unsigned int, 4> fstabColumnWidth(const FstabEntryList& fstabEntries);

struct FstabEntryPrivate
{
    QString m_fsSpec;
    QString m_deviceNode;
    QString m_mountPoint;
    QString m_type;
    QStringList m_options;
    int m_dumpFreq;
    int m_passNumber;
    QString m_comment;
    FstabEntry::Type m_entryType;
};

FstabEntry::FstabEntry(const QString& fsSpec, const QString& mountPoint, const QString& type, const QString& options, int dumpFreq, int passNumber, const QString& comment) :
    d(std::make_unique<FstabEntryPrivate>())
{
    d->m_fsSpec = fsSpec;
    d->m_mountPoint = mountPoint;
    d->m_type = type;
    d->m_dumpFreq = dumpFreq;
    d->m_passNumber = passNumber;
    d->m_comment = comment;

    d->m_options = options.split(QLatin1Char(','));
    d->m_options.removeAll(QStringLiteral("defaults"));
    parseFsSpec(d->m_fsSpec, d->m_entryType, d->m_deviceNode);
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
                fstabEntries.push_back( { {}, {}, {}, {}, {}, {}, line } );
                continue;
            }

            QString comment = line.section( QLatin1Char('#'), 1 );
            QStringList splitLine = line.section( QLatin1Char('#'), 0, 0 ).split( QRegularExpression(QStringLiteral("[\\s]+")), Qt::SkipEmptyParts );

            // We now split the standard components of /etc/fstab entry:
            // (0) path, or UUID, or LABEL, etc,
            // (1) mount point,
            // (2) file system type,
            // (3) options,
            // (4) dump frequency (optional, defaults to 0), no comment is allowed if omitted,
            // (5) pass number (optional, defaults to 0), no comment is allowed if omitted,
            // (#) comment (optional).

            // Handle deprecated subtypes, e.g. sshfs#example. They are not relevant for partitioning, ignore them.
            if (splitLine.size() < 3) {
                continue;
            }

            auto fsSpec = splitLine.at(0);
            auto mountPoint = unescapeSpaces(splitLine.at(1));
            auto fsType = splitLine.at(2);
            // Options may be omitted in some rare cases like NixOS generated fstab.
            auto options = splitLine.length() >= 4 ? splitLine.at(3) : QStringLiteral("defaults");

            switch (splitLine.length()) {
                case 4:
                    fstabEntries.push_back( {fsSpec, mountPoint, fsType, options } );
                    break;
                case 5:
                    fstabEntries.push_back( {fsSpec, mountPoint, fsType, options, splitLine.at(4).toInt() } );
                    break;
                case 6:
                    fstabEntries.push_back( {fsSpec, mountPoint, fsType, options, splitLine.at(4).toInt(), splitLine.at(5).toInt(), comment.isEmpty() ? QString() : QLatin1Char('#') + comment } );
                    break;
                default:
                    fstabEntries.push_back( { {}, {}, {}, {}, {}, {}, QLatin1Char('#') + line } );
            }
        }

        fstabFile.close();
        if (fstabEntries.back().entryType() == FstabEntry::Type::comment && fstabEntries.back().comment().isEmpty())
            fstabEntries.pop_back();
    }

    return fstabEntries;
}

QString escapeSpaces(const QString& mountPoint)
{
    QString tmp = mountPoint;
    tmp.replace(QStringLiteral(" "), QStringLiteral("\\040"));
    tmp.replace(QStringLiteral("\t"), QStringLiteral("\\011"));
    return tmp;
}

QString unescapeSpaces(const QString& mountPoint)
{
    QString tmp = mountPoint;
    tmp.replace(QStringLiteral("\\040"), QStringLiteral(" "));
    tmp.replace(QStringLiteral("\\011"), QStringLiteral("\t"));
    return tmp;
}

void FstabEntry::setFsSpec(const QString& s)
{
    d->m_fsSpec = s;
    parseFsSpec(d->m_fsSpec, d->m_entryType, d->m_deviceNode);
}

const QString& FstabEntry::fsSpec() const
{
    return d->m_fsSpec;
}

const QString& FstabEntry::deviceNode() const
{
    return d->m_deviceNode;
}

const QString& FstabEntry::mountPoint() const
{
    return d->m_mountPoint;
}

const QString& FstabEntry::type() const
{
    return d->m_type;
}

const QStringList& FstabEntry::options() const
{
    return d->m_options;
}

const QString FstabEntry::optionsString() const
{
    return options().size() > 0 ? options().join(QLatin1Char(',')) : QStringLiteral("defaults");
}

int FstabEntry::dumpFreq() const
{
    return d->m_dumpFreq;
}

int FstabEntry::passNumber() const
{
    return d->m_passNumber;
}

const QString& FstabEntry::comment() const
{
    return d->m_comment;
}

FstabEntry::Type FstabEntry::entryType() const
{
    return d->m_entryType;
}

void FstabEntry::setMountPoint(const QString& s)
{
    d->m_mountPoint = s;
}

void FstabEntry::setOptions(const QStringList& s)
{
    d->m_options = s;
}

void FstabEntry::setDumpFreq(int s)
{
    d->m_dumpFreq = s;
}

void FstabEntry::setPassNumber(int s)
{
    d->m_passNumber = s;
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

static QString findBlkIdDevice(const char *token, const QString& value)
{
    QString rval;

#if defined(Q_OS_LINUX)
    if (char* c = blkid_evaluate_tag(token, value.toLocal8Bit().constData(), nullptr)) {
        rval = QString::fromLocal8Bit(c);
        free(c);
    }
#else
    Q_UNUSED(token)
    Q_UNUSED(value)
#endif

    return rval;
}

static void parseFsSpec(const QString& m_fsSpec, FstabEntry::Type& m_entryType, QString& m_deviceNode)
{
    m_entryType = FstabEntry::Type::other;
    m_deviceNode = m_fsSpec;
    if (m_fsSpec.startsWith(QStringLiteral("UUID="))) {
        m_entryType = FstabEntry::Type::uuid;
        m_deviceNode = findBlkIdDevice("UUID", QString(m_fsSpec).remove(QStringLiteral("UUID=")));
    } else if (m_fsSpec.startsWith(QStringLiteral("LABEL="))) {
        m_entryType = FstabEntry::Type::label;
        m_deviceNode = findBlkIdDevice("LABEL", QString(m_fsSpec).remove(QStringLiteral("LABEL=")));
    } else if (m_fsSpec.startsWith(QStringLiteral("PARTUUID="))) {
        m_entryType = FstabEntry::Type::uuid;
        m_deviceNode = findBlkIdDevice("PARTUUID", QString(m_fsSpec).remove(QStringLiteral("PARTUUID=")));
    } else if (m_fsSpec.startsWith(QStringLiteral("PARTLABEL="))) {
        m_entryType = FstabEntry::Type::label;
        m_deviceNode = findBlkIdDevice("PARTLABEL", QString(m_fsSpec).remove(QStringLiteral("PARTLABEL=")));
    } else if (m_fsSpec.startsWith(QStringLiteral("/"))) {
        m_entryType = FstabEntry::Type::deviceNode;
    } else if (m_fsSpec.isEmpty()) {
        m_entryType = FstabEntry::Type::comment;
    }
}


// Used to nicely format fstab file
std::array<unsigned int, 4> fstabColumnWidth(const FstabEntryList& fstabEntries)
{
    std::array<unsigned int, 4> columnWidth;

#define FIELD_WIDTH(x) 3 + escapeSpaces(std::max_element(fstabEntries.begin(), fstabEntries.end(), [](const FstabEntry& a, const FstabEntry& b) {return escapeSpaces(a.x()).length() < escapeSpaces(b.x()).length(); })->x()).length();

    columnWidth[0] = FIELD_WIDTH(fsSpec);
    columnWidth[1] = FIELD_WIDTH(mountPoint);
    columnWidth[2] = FIELD_WIDTH(type);
    columnWidth[3] = FIELD_WIDTH(optionsString);

    return columnWidth;
}

static void writeEntry(QTextStream& s, const FstabEntry& entry, std::array<unsigned int, 4> columnWidth)
{
    if (entry.entryType() == FstabEntry::Type::comment) {
        s << entry.comment() << "\n";
        return;
    }

    // "none" is only valid as mount point for swap partitions
    if ((entry.mountPoint().isEmpty() || entry.mountPoint() == u"none"_s) && entry.type() != QStringLiteral("swap")) {
        return;
    }

    s.setFieldAlignment(QTextStream::AlignLeft);
    s.setFieldWidth(columnWidth[0]);
    s << entry.fsSpec()
      << qSetFieldWidth(columnWidth[1]) << (entry.mountPoint().isEmpty() ? QStringLiteral("none") : escapeSpaces(entry.mountPoint()))
      << qSetFieldWidth(columnWidth[2]) << entry.type()
      << qSetFieldWidth(columnWidth[3]) << entry.optionsString() << qSetFieldWidth(0)
      << entry.dumpFreq() << " "
      << entry.passNumber() << " "
      << entry.comment() << "\n";
}

QString generateFstab(const FstabEntryList& fstabEntries)
{
    QString fstabContents;
    QTextStream out(&fstabContents);

    std::array<unsigned int, 4> columnWidth = fstabColumnWidth(fstabEntries);

    for (const auto &e : fstabEntries)
        writeEntry(out, e, columnWidth);

    out.flush();
    return fstabContents;
}

bool writeMountpoints(const FstabEntryList& fstabEntries)
{
    auto fstab = generateFstab(fstabEntries);

    ExternalCommand cmd;
    return cmd.writeFstab(fstab.toLocal8Bit());
}
