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

#if !defined(KPMCORE_LUKS_H)

#define KPMCORE_LUKS_H

#include "util/libpartitionmanagerexport.h"

#include "fs/filesystem.h"

#include <QtGlobal>
#include <QWidget>

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
    luks(qint64 firstsector, qint64 lastsector, qint64 sectorsused, const QString& label, FileSystem::Type t = FileSystem::Luks);
    ~luks() override;

    enum KeyLocation {
        unknown,
        dmcrypt,
        keyring
    };

public:
    void init() override;
    void scan(const QString& deviceNode) override;
    qint64 readUsedCapacity(const QString& deviceNode) const override;

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
        if (!m_isCryptOpen)
            return cmdSupportNone;
        if (m_Grow && m_innerFs)
            return m_innerFs->supportGrow();
        return cmdSupportNone;
    }
    CommandSupportType supportGrowOnline() const override {
        if (!m_isCryptOpen)
            return cmdSupportNone;
        if (m_Grow && m_innerFs)
            return m_innerFs->supportGrowOnline();
        return cmdSupportNone;
    }
    CommandSupportType supportShrink() const override {
        if (!m_isCryptOpen)
            return cmdSupportNone;
        if (m_Shrink && m_innerFs)
            return m_innerFs->supportShrink();
        return cmdSupportNone;
    }
    CommandSupportType supportShrinkOnline() const override {
        if (!m_isCryptOpen)
            return cmdSupportNone;
        if (m_Shrink && m_innerFs)
            return m_innerFs->supportShrinkOnline();
        return cmdSupportNone;
    }
    CommandSupportType supportMove() const override {
        if (m_isCryptOpen)
            return cmdSupportNone;
        return m_Move;
    }
    CommandSupportType supportCheck() const override {
        if (!m_isCryptOpen)
            return cmdSupportNone;
        if (m_Check && m_innerFs)
            return m_innerFs->supportCheck();
        return cmdSupportNone;
    }
    CommandSupportType supportCheckOnline() const override {
        if (!m_isCryptOpen)
            return cmdSupportNone;
        if (m_Check && m_innerFs)
            return m_innerFs->supportCheckOnline();
        return cmdSupportNone;
    }
    CommandSupportType supportCopy() const override {
        if (m_isCryptOpen)
            return cmdSupportNone;
        return m_Copy;
    }
    CommandSupportType supportBackup() const override {
        return m_Backup;
    }
    CommandSupportType supportSetLabel() const override {
        if (m_Check && m_innerFs)
            return m_innerFs->supportSetLabel();
        return cmdSupportNone;
    }
    CommandSupportType supportUpdateUUID() const override {
        return m_UpdateUUID;
    }
    CommandSupportType supportGetUUID() const override {
        return m_GetUUID;
    }

    bool check(Report& report, const QString& deviceNode) const override;
    bool create(Report& report, const QString& deviceNode) override;
    SupportTool supportToolName() const override;
    bool supportToolFound() const override;
    QString readUUID(const QString& deviceNode) const override;
    bool updateUUID(Report& report, const QString& deviceNode) const override;
    bool resize(Report& report, const QString& deviceNode, qint64 length) const override;
    bool resizeOnline(Report& report, const QString& deviceNode, const QString& mountPoint, qint64 length) const override;
    QString readLabel(const QString& deviceNode) const override;
    bool writeLabel(Report& report, const QString& deviceNode, const QString& newLabel) override;

    QString mountTitle() const override;
    QString unmountTitle() const override;
    QString cryptOpenTitle() const;
    QString cryptCloseTitle() const;

    void setPassphrase(const QString&);
    QString passphrase() const;

    bool canMount(const QString&, const QString&) const override;
    bool canUnmount(const QString&) const override;
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

    bool mount(Report& report, const QString& deviceNode, const QString& mountPoint) override;
    bool unmount(Report& report, const QString& deviceNode) override;

    FileSystem::Type type() const override;

    QString suggestedMapperName(const QString& deviceNode) const;

    void getMapperName(const QString& deviceNode);
    virtual void getLuksInfo(const QString& deviceNode);

    FileSystem* innerFS() const { return m_innerFs; }
    QString outerUuid() const;

    QString mapperName() const { return m_MapperName; }
    QString cipherName() const { return m_CipherName; }
    QString cipherMode() const { return m_CipherMode; }
    QString hashName() const { return m_HashName; }
    qint64 keySize() const { return m_KeySize; }
    qint64 payloadOffset() const { return m_PayloadOffset; }

    static bool canEncryptType(FileSystem::Type type);
    void initLUKS();

    bool testPassphrase(const QString& deviceNode, const QString& passphrase) const;

protected:
    virtual QString readOuterUUID(const QString& deviceNode) const;
    void setPayloadSize();

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

protected:
    mutable FileSystem* m_innerFs;

    mutable bool m_isCryptOpen;
    mutable bool m_cryptsetupFound;
    QString m_passphrase;
    bool m_isMounted;

    QString m_MapperName;
    QString m_CipherName;
    QString m_CipherMode;
    QString m_HashName;
    qint64 m_KeySize;
    qint64 m_PayloadOffset;
    qint64 m_PayloadSize;
    QString m_outerUuid;

    luks::KeyLocation m_KeyLocation = unknown;
};
}

#endif
