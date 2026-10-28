/*
    SPDX-FileCopyrightText: 2026 Ramil Nurmanov <ramil2004nur@gmail.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "ops/trimoperation.h"

#include "core/partition.h"
#include "core/device.h"

#include "fs/luks.h"

#include "jobs/trimjob.h"

#include "util/externalcommand_trustedprefixes.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStorageInfo>
#include <QString>

#include <KLocalizedString>

#include <optional>

static const FileSystem* trimFileSystem(const Partition& p)
{
    const FileSystem* fs = &p.fileSystem();

    if (fs->type() == FileSystem::Type::Luks || fs->type() == FileSystem::Type::Luks2) {
        const FS::luks* luksFs = dynamic_cast<const FS::luks*>(fs);
        if (!luksFs || luksFs->mapperName().isEmpty())
            return nullptr;
        return luksFs->innerFS();
    }

    return fs;
}

static std::optional<bool> discardSupported(const QString& deviceNode)
{
    const QString name = QFileInfo(deviceNode).fileName();
    if (name.isEmpty())
        return std::nullopt;

    const QString sysPath = QFileInfo(QStringLiteral("/sys/class/block/") + name).canonicalFilePath();
    if (sysPath.isEmpty())
        return std::nullopt;

    QFile file(sysPath + QStringLiteral("/queue/discard_max_bytes"));
    if (!file.exists())
        file.setFileName(QFileInfo(sysPath).absolutePath() + QStringLiteral("/queue/discard_max_bytes"));

    if (!file.open(QIODevice::ReadOnly))
        return std::nullopt;

    bool ok = false;
    const quint64 maxBytes = file.readAll().trimmed().toULongLong(&ok);
    if (!ok)
        return std::nullopt;

    return maxBytes > 0;
}

static QString trimDeviceNode(const Partition& p)
{
    const FileSystem& fs = p.fileSystem();
    QString deviceNode = p.deviceNode();

    if (fs.type() == FileSystem::Type::Luks || fs.type() == FileSystem::Type::Luks2) {
        const FS::luks* luksFs = dynamic_cast<const FS::luks*>(&fs);
        deviceNode = luksFs ? luksFs->mapperName() : QString();
    }

    if (deviceNode.isEmpty())
        return QString();

    return QFileInfo(deviceNode).canonicalFilePath();
}

TrimOperation::TrimOperation(Partition& p) :
    Operation(),
    m_TrimmedPartition(p),
    m_TrimJob(new TrimJob(trimmedPartition()))
{
    addJob(trimJob());
}

bool TrimOperation::targets(const Device& d) const
{
    return trimmedPartition().devicePath() == d.deviceNode();
}

bool TrimOperation::targets(const Partition& p) const
{
    return p == trimmedPartition();
}

QString TrimOperation::description() const
{
    return xi18nc("@info:status", "Discard unused blocks on <filename>%1</filename>", trimmedPartition().mountPoint());
}

bool TrimOperation::supportsTrim(FileSystem::Type type)
{
    switch (type) {
    case FileSystem::Type::Ext2:
    case FileSystem::Type::Ext3:
    case FileSystem::Type::Ext4:
    case FileSystem::Type::Btrfs:
    case FileSystem::Type::Xfs:
    case FileSystem::Type::Jfs:
    case FileSystem::Type::F2fs:
    case FileSystem::Type::Nilfs2:
    case FileSystem::Type::Ocfs2:
    case FileSystem::Type::Fat12:
    case FileSystem::Type::Fat16:
    case FileSystem::Type::Fat32:
    case FileSystem::Type::Exfat:
    case FileSystem::Type::Ntfs:
        return true;
    default:
        return false;
    }
}

TrimOperation::Check TrimOperation::check(const Partition& p)
{
    if (findTrustedCommand(QStringLiteral("fstrim")).isEmpty())
        return Check::ToolNotFound;

    const FileSystem* fs = trimFileSystem(p);
    if (fs == nullptr || !supportsTrim(fs->type()))
        return Check::UnsupportedFileSystem;

    const QString mountPoint = p.mountPoint();
    const QString expectedDevice = trimDeviceNode(p);
    if (mountPoint.isEmpty() || expectedDevice.isEmpty())
        return Check::NotMounted;

    const QStorageInfo storage(mountPoint);
    if (!storage.isValid() || !storage.isReady() || QDir::cleanPath(storage.rootPath()) != QDir::cleanPath(mountPoint))
        return Check::NotMounted;

    const QString mountedDevice = QFileInfo(QFile::decodeName(storage.device())).canonicalFilePath();
    if (mountedDevice.isEmpty() || mountedDevice != expectedDevice)
        return Check::NotMounted;

    if (storage.isReadOnly())
        return Check::ReadOnly;

    if (discardSupported(expectedDevice) == false)
        return Check::DiscardUnsupported;

    return Check::Ok;
}

bool TrimOperation::canTrim(const Partition* p)
{
    if (p == nullptr)
        return false;

    return check(*p) == Check::Ok;
}
