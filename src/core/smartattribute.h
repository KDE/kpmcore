/*
    SPDX-FileCopyrightText: 2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2015 Chris Campbell <c.j.campbell@ed.ac.uk>
    SPDX-FileCopyrightText: 2016-2018 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2018 Caio Jordão Carvalho <caiojcarvalho@gmail.com>
    SPDX-FileCopyrightText: 2019 Yuri Chornoivan <yurchor@ukr.net>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KPMCORE_SMARTATTRIBUTE_H
#define KPMCORE_SMARTATTRIBUTE_H

#include "util/libpartitionmanagerexport.h"

#include <QString>

class SmartAttributeParsedData;

class LIBKPMCORE_EXPORT SmartAttribute
{
public:
    enum class FailureType {
        PreFailure,
        OldAge
    };

    enum class UpdateType {
        Online,
        Offline
    };

    enum class Assessment {
        NotApplicable,
        Failing,
        HasFailed,
        Warning,
        Good
    };

public:
    explicit SmartAttribute(const SmartAttributeParsedData& a);

public:
    qint32 id() const {
        return m_Id;
    }
    const QString& name() const {
        return m_Name;
    }
    const QString& desc() const {
        return m_Desc;
    }
    FailureType failureType() const {
        return m_FailureType;
    }
    UpdateType updateType() const {
        return m_UpdateType;
    }
    qint32 current() const {
        return m_Current;
    }
    qint32 worst() const {
        return m_Worst;
    }
    qint32 threshold() const {
        return m_Threshold;
    }
    const QString& raw() const {
        return m_Raw;
    }
    Assessment assessment() const {
        return m_Assessment;
    }
    const QString& value() const {
        return m_Value;
    }

    QString assessmentToString() const {
        return assessmentToString(assessment());
    }
    static QString assessmentToString(Assessment a);

private:
    qint32 m_Id;
    QString m_Name;
    QString m_Desc;
    FailureType m_FailureType;
    UpdateType m_UpdateType;
    qint32 m_Current;
    qint32 m_Worst;
    qint32 m_Threshold;
    QString m_Raw;
    Assessment m_Assessment;
    QString m_Value;
};

#endif
