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

#ifndef KPMCORE_FSFEATURE_H
#define KPMCORE_FSFEATURE_H

#include "util/libpartitionmanagerexport.h"

#include <QString>

#include <memory>

struct FSFeaturePrivate;

/**
 * Class for filesystem-specific features
 *
 * FSFeatures have a name, type (boolean, integer or string) and value.
 * This class can be used to describe specific features for any FileSystem, but it
 * is up to each FileSystem implementation to handle them as needed.
 */
class LIBKPMCORE_EXPORT FSFeature
{
public:
    enum Type : int {
        Bool,
        Int,
        String
    };

public:
    FSFeature(const QString& n, Type t = Type::Bool);
    FSFeature(const QString& n, bool b);
    FSFeature(const QString& n, int i);
    FSFeature(const QString& n, const QString& s);
    FSFeature(const FSFeature& f);
    ~FSFeature();

    FSFeature& operator=(const FSFeature& f);

    Type type();
    const QString& name();

    /**< @return the value of a boolean FSFeature ; feature type is NOT checked */
    bool bValue();
    /**< @return the value of an integer FSFeature ; feature type is NOT checked */
    int iValue();
    /**< @return the value of a string FSFeature ; feature type is NOT checked */
    const QString& sValue();

    /**
     * Set feature value
     * @return false if the feature is of the wrong type
     */
    bool setValue(bool b);
    bool setValue(int i);
    bool setValue(const QString& s);

private:
    std::unique_ptr<FSFeaturePrivate> d;
};

#endif
