/*************************************************************************
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

#if !defined(KPMCORE_ACTIVATERAIDOPERATION_H)

#define KPMCORE_ACTIVATERAIDOPERATION_H

#include "ops/operation.h"

class ActivateRaidJob;
class Device;
class Partition;
class SoftwareRAID;

class LIBKPMCORE_EXPORT ActivateRaidOperation : public Operation
{
    Q_DISABLE_COPY(ActivateRaidOperation)
    
public:
    ActivateRaidOperation(SoftwareRAID* raid);
    
public:
    QString iconName() const override { return QStringLiteral("answer"); }
    
    QString description() const override;
    
    virtual bool targets(const Device& device) const override;
    
    virtual bool targets(const Partition&) const override {
        return true;
    }
    
    virtual void preview() override;
    virtual void undo() override;
    
protected:
    ActivateRaidJob* activateRaidJob() const {
        return m_activateRaidJob;
    }
    
private:
    ActivateRaidJob* m_activateRaidJob;
    SoftwareRAID* m_raid;
    
};

#endif // KPMCORE_ACTIVATERAIDOPERATION_H
