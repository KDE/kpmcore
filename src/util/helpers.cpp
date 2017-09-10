/*************************************************************************
 *  Copyright (C) 2008, 2009, 2010 by Volker Lanz <vl@fidra.de>          *
 *  Copyright (C) 2016 by Andrius Štikonas <andrius@stikonas.eu>         *
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

#include "util/helpers.h"
#include "util/externalcommand.h"
#include "util/globallog.h"

#include "ops/operation.h"

#include <KAboutData>
#include <KLocalizedString>

#include <QAction>
#include <QMenu>
#include <QHeaderView>
#include <QRect>
#include <QTreeWidget>

void registerMetaTypes()
{
    qRegisterMetaType<Operation*>("Operation*");
    qRegisterMetaType<Log::Level>("Log::Level");
}

bool caseInsensitiveLessThan(const QString& s1, const QString& s2)
{
    return s1.toLower() < s2.toLower();
}

void showColumnsContextMenu(const QPoint& p, QTreeWidget& tree)
{
    QMenu headerMenu(xi18nc("@title:menu", "Columns"));

    QHeaderView* header = tree.header();

    for (qint32 i = 0; i < tree.model()->columnCount(); i++) {
        const int idx = header->logicalIndex(i);
        const QString text = tree.model()->headerData(idx, Qt::Horizontal).toString();

        QAction* action = headerMenu.addAction(text);
        action->setCheckable(true);
        action->setChecked(!header->isSectionHidden(idx));
        action->setData(idx);
        action->setEnabled(idx > 0);
    }

    QAction* action = headerMenu.exec(tree.header()->mapToGlobal(p));

    if (action != nullptr) {
        const bool hidden = !action->isChecked();
        tree.setColumnHidden(action->data().toInt(), hidden);
        if (!hidden)
            tree.resizeColumnToContents(action->data().toInt());
    }
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
                          KAboutLicense::GPL_V3, xi18nc("@info:credit", "&copy; 2008-2017 KPMcore developers" ) );
    aboutData.setOrganizationDomain(QByteArray("kde.org"));
    aboutData.setProductName(QByteArray("kpmcore"));
    aboutData.setHomepage(QStringLiteral("https://commits.kde.org/kpmcore"));

    aboutData.addAuthor(xi18nc("@info:credit", "Volker Lanz"), xi18nc("@info:credit", "Former maintainer"));
    aboutData.addAuthor(xi18nc("@info:credit", "Andrius Štikonas"), xi18nc("@info:credit", "Maintainer"), QStringLiteral("andrius@stikonas.eu"));
    aboutData.addAuthor(xi18nc("@info:credit", "Teo Mrnjavac"), i18nc("@info:credit", "Calamares maintainer"), QStringLiteral("teo@kde.org"));
    aboutData.addAuthor(xi18nc("@info:credit", "Chantara Tith"), i18nc("@info:credit", "LVM support"), QStringLiteral("tith.chantara@gmail.com"));
    aboutData.addAuthor(xi18nc("@info:credit", "Pali Rohár"), i18nc("@info:credit", "UDF support"), QStringLiteral("pali.rohar@gmail.com"));

    return aboutData;
}
