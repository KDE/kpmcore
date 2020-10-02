/*
    SPDX-FileCopyrightText: 2008-2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2015 Teo Mrnjavac <teo@kde.org>
    SPDX-FileCopyrightText: 2016-2018 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2018 Huzaifa Faruqui <huzaifafaruqui@gmail.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KPMCORE_COPYSOURCEDEVICE_H
#define KPMCORE_COPYSOURCEDEVICE_H

#include "backend/corebackenddevice.h"
#include "core/copysource.h"
#include "util/libpartitionmanagerexport.h"

#include <memory>

#include <QtGlobal>

class Device;
class CopyTarget;
class CoreBackendDevice;
class QString;

/** A Device to copy from.

    Represents a Device to copy from. Used to copy a Partition to somewhere on the same or
    another Device or to backup its FileSystem to a file.
    @author Volker Lanz <vl@fidra.de>
 */
class CopySourceDevice : public CopySource
{
    Q_DISABLE_COPY(CopySourceDevice)

public:
    CopySourceDevice(Device& d, qint64 firstbyte, qint64 lastbyte);

public:
    bool open() override;
    qint64 length() const override;
    bool overlaps(const CopyTarget& target) const override;

    qint64 firstByte() const override {
        return m_FirstByte;    /**< @return first byte to copy */
    }
    qint64 lastByte() const override {
        return m_LastByte;    /**< @return last byte to copy */
    }

    Device& device() {
        return m_Device;    /**< @return Device to copy from */
    }
    const Device& device() const {
        return m_Device;    /**< @return Device to copy from */
    }

    QString path() const override;

protected:
    Device& m_Device;
    const qint64 m_FirstByte;
    const qint64 m_LastByte;
    std::unique_ptr<CoreBackendDevice> m_BackendDevice;
};

#endif
