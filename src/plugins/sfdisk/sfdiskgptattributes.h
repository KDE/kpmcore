/*************************************************************************
  *  Copyright (C) 2020 by Gaël PORTAY <gael.portay@collabora.com>       *
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

#if !defined(SFDISKGPTATTRIBUTES__H)

#define SFDISKGPTATTRIBUTES__H

#include <QtGlobal>

class QStringList;

/** Sfdisk GPT Attributes helpers.
    @author Gaël PORTAY <gael.portay@collabora.com>
*/
class SfdiskGptAttributes
{
public:
    static quint64 toULongLong(const QStringList& attrs);
    static QStringList toStringList(quint64 attrs);
};

#endif
