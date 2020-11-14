#! /usr/bin/env bash

# SPDX-FileCopyrightText: 2008 Laurent Montel <montel@kde.org>

# SPDX-License-Identifier: MIT

$XGETTEXT $(find -name \*.cc -o -name \*.cpp -o -name \*.h$) -o $podir/kpmcore.pot
