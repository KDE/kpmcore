/*
    SPDX-FileCopyrightText: 2026 Ramil Nurmanov <ramil2004nur@gmail.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KPMCORE_TRIMOPERATION_H
#define KPMCORE_TRIMOPERATION_H

#include "util/libpartitionmanagerexport.h"

#include "fs/filesystem.h"
#include "ops/operation.h"

#include <QString>

class Partition;
class TrimJob;

class LIBKPMCORE_EXPORT TrimOperation : public Operation
{
    Q_DISABLE_COPY(TrimOperation)

public:
    enum class Check {
        Ok,
        UnsupportedFileSystem,
        NotMounted,
        ReadOnly,
        DiscardUnsupported,
    };

    explicit TrimOperation(Partition& p);

public:
    QString iconName() const override {
        return QStringLiteral("edit-clear-all");
    }
    QString description() const override;
    void preview() override {}
    void undo() override {}

    bool targets(const Device& d) const override;
    bool targets(const Partition& p) const override;

    static bool supportsTrim(FileSystem::Type type);
    static QString trimDeviceNode(const Partition& p);
    static Check check(const Partition& p);
    static bool canTrim(const Partition* p);

protected:
    Partition& trimmedPartition() {
        return m_TrimmedPartition;
    }
    const Partition& trimmedPartition() const {
        return m_TrimmedPartition;
    }

    TrimJob* trimJob() {
        return m_TrimJob;
    }

private:
    Partition& m_TrimmedPartition;
    TrimJob* m_TrimJob;
};

#endif
