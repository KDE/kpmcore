/*************************************************************************
 *  Copyright (C) 2019 by Collabora Ltd <arnaud.ferraris@collabora.com>  *
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

#include "fs/filesystem.h"

struct FSFeaturePrivate {
    QString m_name;
    FSFeature::Type m_type;
    bool m_boolean;
    int m_integer;
    QString m_string;
};

FSFeature::FSFeature(const QString& n, FSFeature::Type t) :
    d(std::make_unique<FSFeaturePrivate>())
{
    d->m_name = n;
    d->m_type = t;
}

FSFeature::FSFeature(const QString& n, bool b) :
    d(std::make_unique<FSFeaturePrivate>())
{
    d->m_name = n;
    d->m_type = FSFeature::Type::Bool;
    d->m_boolean = b;
}

FSFeature::FSFeature(const QString& n, int i) :
    d(std::make_unique<FSFeaturePrivate>())
{
    d->m_name = n;
    d->m_type = FSFeature::Type::Int;
    d->m_integer = i;
}

FSFeature::FSFeature(const QString& n, const QString& s) :
    d(std::make_unique<FSFeaturePrivate>())
{
    d->m_name = n;
    d->m_type = FSFeature::Type::String;
    d->m_string = s;
}

FSFeature::FSFeature(const FSFeature& other) :
    d(std::make_unique<FSFeaturePrivate>(*(other.d)))
{
}

FSFeature::~FSFeature()
{
}

FSFeature& FSFeature::operator=(const FSFeature& other)
{
    if (&other != this)
        *d = *(other.d);

    return *this;
}

const QString& FSFeature::name()
{
    return d->m_name;
}

FSFeature::Type FSFeature::type()
{
    return d->m_type;
}

bool FSFeature::bValue()
{
    return d->m_boolean;
}

int FSFeature::iValue()
{
    return d->m_integer;
}

const QString& FSFeature::sValue()
{
    return d->m_string;
}

bool FSFeature::setValue(bool b)
{
    if (d->m_type != FSFeature::Type::Bool)
        return false;

    d->m_boolean = b;
    return true;
}

bool FSFeature::setValue(int i)
{
    if (d->m_type != FSFeature::Type::Int)
        return false;

    d->m_integer = i;
    return true;
}

bool FSFeature::setValue(const QString& s)
{
    if (d->m_type != FSFeature::Type::String)
        return false;

    d->m_string = s;
    return true;
}
