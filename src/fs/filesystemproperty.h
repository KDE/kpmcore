/*
    SPDX-FileCopyrightText: 2026 Ramil Nurmanov <ramil2004nur@gmail.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KPMCORE_FILESYSTEMPROPERTY_H
#define KPMCORE_FILESYSTEMPROPERTY_H

#include "util/libpartitionmanagerexport.h"

#include <QString>
#include <QVariant>

class LIBKPMCORE_EXPORT FileSystemProperty
{
public:
    enum class DisplayType {
        Text,
        Number,
        Bytes,
        Percent,
        List,
    };

    enum class Group {
        UnitSizes,
        Capabilities,
        Reserved,
        Metadata,
        Journal,
        Specific,
    };

    FileSystemProperty() = default;
    FileSystemProperty(const QString& propertyId, const QVariant& propertyValue,
                       DisplayType type, Group propertyGroup)
        : id(propertyId)
        , value(propertyValue)
        , displayType(type)
        , group(propertyGroup)
    {
    }

    QString id;
    QVariant value;
    DisplayType displayType = DisplayType::Text;
    Group group = Group::Specific;
};

#endif
