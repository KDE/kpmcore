/*
    SPDX-FileCopyrightText: 2026 Ramil Nurmanov <ramil2004nur@gmail.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KPMCORE_SMARTDETAILS_H
#define KPMCORE_SMARTDETAILS_H

#include "util/libpartitionmanagerexport.h"
#include "core/smartnvmehealth.h"

#include <QString>

#include <memory>
#include <optional>

class SmartDetailsPrivate;

class LIBKPMCORE_EXPORT SmartDetails
{
    Q_DISABLE_COPY(SmartDetails)

public:
    explicit SmartDetails(const QString &devicePath);
    ~SmartDetails();

public:
    void update();

    const QString &devicePath() const;
    bool isValid() const;
    bool isNvme() const;

    std::optional<quint64> temp() const;
    std::optional<quint64> poweredOn() const;
    std::optional<quint64> powerCycles() const;
    std::optional<quint64> badSectors() const;

    const SmartNvmeHealth &nvmeHealth() const;

private:
    std::unique_ptr<SmartDetailsPrivate> d;
};

#endif
