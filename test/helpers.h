/*
    SPDX-FileCopyrightText: 2017 by Adriaan de Groot <groot@kde.org>
    SPDX-FileCopyrightText: 2018 Andrius Štikonas <andrius@stikonas.eu>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef TEST_KPMHELPERS_H
#define TEST_KPMHELPERS_H

class QString;

/**
 * Use RAII to initialize the KPMcore library. Just instantiate one
 * object of this class to do "normal" initialization.
 */
class KPMCoreInitializer
{
public:
    KPMCoreInitializer(); /// Default backend
    KPMCoreInitializer( const QString& backend );  /// Use named backend
    KPMCoreInitializer( const char* backend );  /// Use named backend

    bool isValid() const
    {
        return m_isValid;
    }
private:
    bool m_isValid;
} ;

#endif
