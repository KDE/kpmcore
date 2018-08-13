/*************************************************************************
 *  Copyright (C) 2016 by Chantara Tith <tith.chantara@gmail.com>        *
 *  Copyright (C) 2016 by Andrius Štikonas <andrius@stikonas.eu>         *
 *  Copyright (C) 2018 by Caio Carvalho <caiojcarvalho@gmail.com>        *
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

#if !defined(KPMCORE_CREATEVOLUMEGROUPOPERATION_H)

#define KPMCORE_CREATEVOLUMEGROUPOPERATION_H

#include "util/libpartitionmanagerexport.h"

#include "ops/operation.h"

#include "core/lvmdevice.h"
#include "core/volumemanagerdevice.h"

#include <QString>

class CreateVolumeGroupJob;
class OperationStack;

class LIBKPMCORE_EXPORT CreateVolumeGroupOperation : public Operation
{
    Q_DISABLE_COPY(CreateVolumeGroupOperation)

    friend class OperationStack;

public:
    CreateVolumeGroupOperation(const QString& vgName, const QVector<const Partition*>& pvList,
                               const Device::Type type, const qint32 peSize = 4);

    CreateVolumeGroupOperation(const QString& vgName, const QVector<const Partition*>& pvList,
                               const Device::Type type, const qint32 raidLevel,
                               const qint32 chunkSize);

public:
    QString iconName() const override {
        return QStringLiteral("document-new");
    }

    QString description() const override;

    virtual bool targets(const Device&) const override {
        return true;
    }
    virtual bool targets(const Partition&) const override;

    virtual void preview() override;
    virtual void undo() override;

    static bool canCreate();

protected:
    CreateVolumeGroupJob* createVolumeGroupJob() const {
        return m_CreateVolumeGroupJob;
    }

    const QVector<const Partition*>& PVList() const {
        return m_PVList;
    }

    Device::Type type() const {
        return m_type;
    }

private:
    CreateVolumeGroupJob* m_CreateVolumeGroupJob;
    const QVector<const Partition*> m_PVList;
    QString m_vgName;
    Device::Type m_type;
};

#endif
