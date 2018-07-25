/*************************************************************************
 *  Copyright 2017 by Adriaan de Groot <groot@kde.org>                   *
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

//  SPDX-License-Identifier: GPL-3.0+

// Initializes KPMcore, and either loads the default backend for
// the current platform, or if one is named on the command line,
// loads that one. Returns 0 on success.

#include <QCoreApplication>

#include "helpers.h"

int main( int argc, char** argv )
{
    QCoreApplication app(argc, argv);
    if ( argc != 2 )
    {
        KPMCoreInitializer i;
        return i.isValid() ? 0 : 1;
    }
    else
    {
        KPMCoreInitializer i( argv[1] );
        return i.isValid() ? 0 : 1;
    }
}

