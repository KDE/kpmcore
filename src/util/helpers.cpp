/*
    SPDX-FileCopyrightText: 2008-2012 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2012-2018 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2015-2016 Teo Mrnjavac <teo@kde.org>
    SPDX-FileCopyrightText: 2016 Chantara Tith <tith.chantara@gmail.com>
    SPDX-FileCopyrightText: 2016 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "util/helpers.h"
#include "util/externalcommand.h"
#include "util/globallog.h"

#include "ops/operation.h"

#include <KAboutData>
#include <KLocalizedString>

void registerMetaTypes()
{
    qRegisterMetaType<Operation*>("Operation*");
    qRegisterMetaType<Log::Level>("Log::Level");
}

bool caseInsensitiveLessThan(const QString& s1, const QString& s2)
{
    return s1.toLower() < s2.toLower();
}

bool isMounted(const QString& deviceNode)
{
    ExternalCommand cmd(QStringLiteral("lsblk"),
                        { QStringLiteral("--noheadings"),
                          QStringLiteral("--nodeps"),
                          QStringLiteral("--output"),
                          QStringLiteral("mountpoint"),
                          deviceNode });

    if (cmd.run(-1) && cmd.exitCode() == 0) {
        return !cmd.output().trimmed().isEmpty();
    }
    return false;
}

KAboutData aboutKPMcore()
{
    KAboutData aboutData( QStringLiteral("kpmcore"),
                          xi18nc("@title", "<application>KPMcore</application>"), QStringLiteral(VERSION),
                          xi18nc("@title", "Library for managing partitions"),
                          KAboutLicense::GPL_V3, xi18nc("@info:credit", "&copy; 2008-2020 KPMcore developers" ) );
    aboutData.setOrganizationDomain(QByteArray("kde.org"));
    aboutData.setProductName(QByteArray("kpmcore"));
    aboutData.setHomepage(QStringLiteral("https://commits.kde.org/kpmcore"));

    aboutData.addAuthor(xi18nc("@info:credit", "Volker Lanz"), xi18nc("@info:credit", "Former maintainer"));
    aboutData.addAuthor(xi18nc("@info:credit", "Andrius Štikonas"), xi18nc("@info:credit", "Maintainer"), QStringLiteral("andrius@stikonas.eu"));
    aboutData.addCredit(xi18nc("@info:credit", "Teo Mrnjavac"), i18nc("@info:credit", "Former Calamares maintainer"), QStringLiteral("teo@kde.org"));
    aboutData.addCredit(xi18nc("@info:credit", "Chantara Tith"), i18nc("@info:credit", "LVM support"), QStringLiteral("tith.chantara@gmail.com"));
    aboutData.addCredit(xi18nc("@info:credit", "Pali Rohár"), i18nc("@info:credit", "UDF support"), QStringLiteral("pali.rohar@gmail.com"));
    aboutData.addCredit(xi18nc("@info:credit", "Adriaan de Groot"), i18nc("@info:credit", "Calamares maintainer"), QStringLiteral("groot@kde.org"));
    aboutData.addCredit(xi18nc("@info:credit", "Caio Jordão Carvalho"), i18nc("@info:credit", "Improved SMART support"), QStringLiteral("caiojcarvalho@gmail.com"));
    aboutData.addCredit(xi18nc("@info:credit", "David Edmundson"), i18nc("@info:credit", "Port from KAuth to Polkit"), QStringLiteral("kde@davidedmundson.co.uk"));

    return aboutData;
}
