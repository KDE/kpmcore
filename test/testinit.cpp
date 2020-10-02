/*
    SPDX-FileCopyrightText: 2017 Adriaan de Groot <groot@kde.org>
    SPDX-FileCopyrightText: 2018 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2019 Shubham Jangra <aryan100jangid@gmail.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

// Initializes KPMcore, and either loads the default backend for
// the current platform, or if one is named on the command line,
// loads that one. Returns 0 on success.

#include <QCoreApplication>

#include "helpers.h"

int main( int argc, char** argv )
{
    QCoreApplication app(argc, argv);
    if ( argc != 2 ) {
        KPMCoreInitializer i;
        return i.isValid() ? 0 : 1;
    } else {
        KPMCoreInitializer i( argv[1] );
        return i.isValid() ? 0 : 1;
    }
}

