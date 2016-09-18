/*************************************************************************
 *  Copyright (C) 2012 by Volker Lanz <vl@fidra.de>                      *
 *  Copyright (C) 2016 by Andrius Štikonas <andrius@stikonas.eu>         *
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

#if !defined(LVM2_PV__H)

#define LVM2_PV__H

#include "util/libpartitionmanagerexport.h"

#include "core/partition.h"
#include "fs/filesystem.h"

#include <QtGlobal>

class Report;

class QString;

namespace FS
{
/** LVM2 physical volume.
    @author Andrius Štikonas <andrius@stikonas.eu>
*/
class LIBKPMCORE_EXPORT lvm2_pv : public FileSystem
{
public:
    typedef QList<QPair<QString, const Partition *>> PhysicalVolumes;

public:
    lvm2_pv(qint64 firstsector, qint64 lastsector, qint64 sectorsused, const QString& label);

public:
    void init() override;
    void scan(const QString& deviceNode) override;

    qint64 readUsedCapacity(const QString& deviceNode) const override;
    bool check(Report& report, const QString& deviceNode) const override;
    bool create(Report& report, const QString& deviceNode) override;
    bool remove(Report& report, const QString& deviceNode) const override;
    bool resize(Report& report, const QString& deviceNode, qint64 length) const override;
//          bool writeLabel(Report& report, const QString& deviceNode, const QString& newLabel) override;
    bool updateUUID(Report& report, const QString& deviceNode) const override;
    QString readUUID(const QString& deviceNode) const override;

    bool canMount(const QString& deviceNode, const QString& mountPoint) const override;
    bool canUnmount(const QString& deviceNode) const override;

    bool mount(Report& report, const QString& deviceNode, const QString& mountPoint) override; // mountPoint == VG name
    bool unmount(Report& report, const QString& deviceNode) override;


    CommandSupportType supportGetUsed() const override {
        return m_GetUsed;
    }
    CommandSupportType supportGetLabel() const override {
        return m_GetLabel;
    }
    CommandSupportType supportCreate() const override {
        return m_Create;
    }
    CommandSupportType supportGrow() const override {
        return m_Grow;
    }
    CommandSupportType supportShrink() const override {
        return m_Shrink;
    }
    CommandSupportType supportMove() const override {
        return m_Move;
    }
    CommandSupportType supportCheck() const override {
        return m_Check;
    }
    CommandSupportType supportCopy() const override {
        return m_Copy;
    }
    CommandSupportType supportBackup() const override {
        return m_Backup;
    }
    CommandSupportType supportSetLabel() const override {
        return m_SetLabel;
    }
    CommandSupportType supportUpdateUUID() const override {
        return m_UpdateUUID;
    }
    CommandSupportType supportGetUUID() const override {
        return m_GetUUID;
    }

    qint64 maxCapacity() const override;
    SupportTool supportToolName() const override;
    bool supportToolFound() const override;

    static QString getpvField(const QString& fieldName, const QString& deviceNode = QString());

    static qint64 getTotalPE(const QString& deviceNode);
    static qint64 getFreePE(const QString& deviceNode);
    static qint64 getAllocatedPE(const QString& deviceNode);
    static QString getVGName(const QString& deviceNode);
    static PhysicalVolumes getPVinNode(const PartitionNode* parent);
    static PhysicalVolumes getPVs(const QList<Device*>& devices);

    qint64 allocatedPE() const { return m_AllocatedPE; };
    qint64 freePE() const { return m_TotalPE - m_AllocatedPE; };
    qint64 totalPE() const { return m_TotalPE; };
    qint64 peSize() const { return m_PESize; };

private:
    void getPESize(const QString& deviceNode); // return PE size in bytes

public:
    static CommandSupportType m_GetUsed;
    static CommandSupportType m_GetLabel;
    static CommandSupportType m_Create;
    static CommandSupportType m_Grow;
    static CommandSupportType m_Shrink;
    static CommandSupportType m_Move;
    static CommandSupportType m_Check;
    static CommandSupportType m_Copy;
    static CommandSupportType m_Backup;
    static CommandSupportType m_SetLabel;
    static CommandSupportType m_UpdateUUID;
    static CommandSupportType m_GetUUID;

private:
    qint64 m_PESize;
    qint64 m_TotalPE;
    qint64 m_AllocatedPE;
};
}

#endif
