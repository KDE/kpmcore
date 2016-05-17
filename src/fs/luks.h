/*************************************************************************
 *  Copyright (C) 2012 by Volker Lanz <vl@fidra.de>                      *
 *  Copyright (C) 2013 by Andrius Štikonas <andrius@stikonas.eu>         *
 *  Copyright (C) 2015-2016 by Teo Mrnjavac <teo@kde.org>                *
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

#if !defined(LUKS__H)

#define LUKS__H

#include "util/libpartitionmanagerexport.h"

#include "fs/filesystem.h"

#include <QtGlobal>
#include <QPointer>

class Report;

class QString;

namespace FS
{
/** A LUKS crypto file system.
    @author Andrius Štikonas <andrius@stikonas.eu>
*/
class LIBKPMCORE_EXPORT luks : public FileSystem
{
public:
    luks(qint64 firstsector, qint64 lastsector, qint64 sectorsused, const QString& label);
    virtual ~luks();

public:
    void init() override;
    virtual qint64 readUsedCapacity(const QString& deviceNode) const override;

    virtual CommandSupportType supportGetUsed() const override {
        return m_GetUsed;
    }
    virtual CommandSupportType supportGetLabel() const override {
        return m_GetLabel;
    }
    virtual CommandSupportType supportCreate() const override {
        return m_Create;
    }
    virtual CommandSupportType supportGrow() const override {
        if (!m_isCryptOpen)
            return cmdSupportNone;
        if (m_Grow && m_innerFs)
            return m_innerFs->supportGrow();
        return cmdSupportNone;
    }
    virtual CommandSupportType supportShrink() const override {
        if (!m_isCryptOpen)
            return cmdSupportNone;
        if (m_Shrink && m_innerFs)
            return m_innerFs->supportShrink();
        return cmdSupportNone;
    }
    virtual CommandSupportType supportMove() const override {
        return m_Move;
    }
    virtual CommandSupportType supportCheck() const override {
        if (!m_isCryptOpen)
            return cmdSupportNone;
        if (m_Check && m_innerFs)
            return m_innerFs->supportCheck();
        return cmdSupportNone;
    }
    virtual CommandSupportType supportCopy() const override {
        return m_Copy;
    }
    virtual CommandSupportType supportBackup() const override {
        return m_Backup;
    }
    virtual CommandSupportType supportSetLabel() const override {
        return m_SetLabel;
    }
    virtual CommandSupportType supportUpdateUUID() const override {
        return m_UpdateUUID;
    }
    virtual CommandSupportType supportGetUUID() const override {
        return m_GetUUID;
    }

    void setLogicalSectorSize(unsigned int logicalSectorSize) {
        m_logicalSectorSize = logicalSectorSize;
    }

    virtual bool check(Report& report, const QString& deviceNode) const override;
    virtual bool create(Report &report, const QString &deviceNode) const override;
    virtual SupportTool supportToolName() const override;
    virtual bool supportToolFound() const override;
    virtual QString readUUID(const QString& deviceNode) const override;
    virtual bool updateUUID(Report& report, const QString& deviceNode) const override;
    virtual bool resize(Report& report, const QString& deviceNode, qint64 length) const override;
    virtual QString readLabel(const QString& deviceNode) const override;
    virtual bool writeLabel(Report& report, const QString& deviceNode, const QString& newLabel) override;

    virtual QString mountTitle() const override;
    virtual QString unmountTitle() const override;
    QString cryptOpenTitle() const;
    QString cryptCloseTitle() const;

    void setPassphrase(const QString&);
    QString passphrase() const;

    virtual bool canMount(const QString&, const QString&) const override;
    virtual bool canUnmount(const QString&) const override;
    bool isMounted() const;
    void setMounted(bool mounted);

    bool canCryptOpen(const QString& deviceNode) const;
    bool canCryptClose(const QString& deviceNode) const;
    bool isCryptOpen() const;
    void setCryptOpen(bool cryptOpen);

    bool cryptOpen(QWidget* parent, const QString& deviceNode);
    bool cryptClose(const QString& deviceNode);

    void loadInnerFileSystem(const QString& mapperNode);
    void createInnerFileSystem(Type type);

    virtual bool mount(Report& report, const QString& deviceNode, const QString& mountPoint) override;
    virtual bool unmount(Report& report, const QString& deviceNode) override;

    virtual FileSystem::Type type() const override;

    QString suggestedMapperName(const QString& deviceNode) const;

    static QString mapperName(const QString& deviceNode);

    static QString getCipherName(const QString& deviceNode);
    static QString getCipherMode(const QString& deviceNode);
    static QString getHashName(const QString& deviceNode);
    static QString getKeySize(const QString& deviceNode);
    static QString getPayloadOffset(const QString& deviceNode);
    static bool canEncryptType(FileSystem::Type type);

protected:
    virtual QString readOuterUUID(const QString& deviceNode) const;

public:
    CommandSupportType m_GetUsed;
    CommandSupportType m_GetLabel;
    CommandSupportType m_Create;
    CommandSupportType m_Grow;
    CommandSupportType m_Shrink;
    CommandSupportType m_Move;
    CommandSupportType m_Check;
    CommandSupportType m_Copy;
    CommandSupportType m_Backup;
    CommandSupportType m_SetLabel;
    CommandSupportType m_UpdateUUID;
    CommandSupportType m_GetUUID;

private:
    mutable FileSystem* m_innerFs;

    mutable bool m_isCryptOpen;
    mutable bool m_cryptsetupFound;
    QString m_passphrase;
    bool m_isMounted;
    unsigned int m_logicalSectorSize;
};
}

#endif
