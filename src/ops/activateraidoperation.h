/*
    SPDX-FileCopyrightText: 2018 Caio Jordão Carvalho <caiojcarvalho@gmail.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KPMCORE_ACTIVATERAIDOPERATION_H
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
