/*************************************************************************
 *  Copyright (C) 2016 by Chantara Tith <tith.chantara@gmail.com>        *
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

#if !defined(REMOVEVOLUMEOPERATION_H)

#define REMOVEVOLUMEGROUPOPERATION_H

#include "util/libpartitionmanagerexport.h"

#include "ops/operation.h"

#include <QString>

class RemoveVolumeGroupJob;
class VolumeManagerDevice;
class OperationStack;

class LIBKPMCORE_EXPORT RemoveVolumeGroupOperation : public Operation
{
    Q_DISABLE_COPY(RemoveVolumeGroupOperation)

    friend class OperationStack;

public:
    RemoveVolumeGroupOperation(VolumeManagerDevice& dev);

public:
    QString iconName() const override {
        return QStringLiteral("edit-delete");
    }

    QString description() const override;

    virtual bool targets(const Device&) const override {
        return true;
    }
    virtual bool targets(const Partition&) const override {
        return false;
    }

    virtual void preview() override;
    virtual void undo() override;

    static bool canRemove();

protected:
    RemoveVolumeGroupJob* removeVolumeGroupJob() {
        return m_RemoveVolumeGroupJob;
    }

private:
    RemoveVolumeGroupJob* m_RemoveVolumeGroupJob;
};

#endif
