/*************************************************************************
 *  Copyright (C) 2008, 2009, 2010 by Volker Lanz <vl@fidra.de>          *
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
#include "../util/globallog.h"

#include "../ops/operation.h"

#include <KAboutData>
#include <KLocalizedString>

#include <QAction>
#include <QIcon>
#include <QMenu>
#include <QHeaderView>
#include <QPainter>
#include <QPixmap>
#include <QRect>
#include <QTreeWidget>

#include <config.h>

void registerMetaTypes()
{
	qRegisterMetaType<Operation*>("Operation*");
	qRegisterMetaType<Log::Level>("Log::Level");
}

bool caseInsensitiveLessThan(const QString& s1, const QString& s2)
{
	return s1.toLower() < s2.toLower();
}

QIcon createFileSystemColor(FileSystem::Type type, quint32 size)
{
	QPixmap pixmap(size, size);
	QPainter painter(&pixmap);
	painter.setPen(QColor(0, 0, 0));
	painter.setBrush(Config::fileSystemColorCode(type));
	painter.drawRect(QRect(0, 0, pixmap.width() - 1, pixmap.height() - 1));
	painter.end();

	return QIcon(pixmap);
}

void showColumnsContextMenu(const QPoint& p, QTreeWidget& tree)
{
	QMenu headerMenu(i18nc("@title:menu", "Columns"));

	QHeaderView* header = tree.header();

	for (qint32 i = 0; i < tree.model()->columnCount(); i++)
	{
		const int idx = header->logicalIndex(i);
		const QString text = tree.model()->headerData(idx, Qt::Horizontal).toString();

		QAction* action = headerMenu.addAction(text);
		action->setCheckable(true);
		action->setChecked(!header->isSectionHidden(idx));
		action->setData(idx);
		action->setEnabled(idx > 0);
	}

	QAction* action = headerMenu.exec(tree.header()->mapToGlobal(p));

	if (action != NULL)
	{
		const bool hidden = !action->isChecked();
		tree.setColumnHidden(action->data().toInt(), hidden);
		if (!hidden)
			tree.resizeColumnToContents(action->data().toInt());
	}
}
