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

#if !defined(KPMCORE_SOFTWARERAID_H)
#define KPMCORE_SOFTWARERAID_H

#include "core/volumemanagerdevice.h"
#include "util/libpartitionmanagerexport.h"
#include "util/report.h"

class LIBKPMCORE_EXPORT SoftwareRAID : public VolumeManagerDevice
{
    Q_DISABLE_COPY(SoftwareRAID)

public:
    enum class Status {
        Active,
        Inactive,
        Resync,
        Recovery,
    };

    SoftwareRAID(const QString& name,
                 SoftwareRAID::Status status = SoftwareRAID::Status::Active,
                 const QString& iconName = QString());

    const QStringList deviceNodes() const override;
    const QStringList& partitionNodes() const override;
    qint64 partitionSize(QString &partitionPath) const override;

    virtual bool growArray(Report& report, const QStringList& devices);

    virtual bool shrinkArray(Report& report, const QStringList& devices);

    virtual QString prettyName() const override;

    virtual bool operator==(const Device& other) const override;

    qint32 raidLevel() const;
    qint64 chunkSize() const;
    qint64 totalChunk() const;
    qint64 arraySize() const;
    QString uuid() const;
    QStringList devicePathList() const;
    SoftwareRAID::Status status() const;

    void setStatus(SoftwareRAID::Status status);

public:
    static void scanSoftwareRAID(QList<Device*>& devices);

    static qint32 getRaidLevel(const QString& path);
    static qint64 getChunkSize(const QString& path);
    static qint64 getTotalChunk(const QString& path);
    static qint64 getArraySize(const QString& path);
    static QString getUUID(const QString& path);
    static QStringList getDevicePathList(const QString& path);

    static bool isRaidPath(const QString& path);

    static bool createSoftwareRAID(Report& report,
                                   const QString& name,
                                   const QStringList devicePathList,
                                   const qint32 raidLevel,
                                   const qint32 chunkSize);

    static bool deleteSoftwareRAID(Report& report,
                                   SoftwareRAID& raidDevice);

    static bool assembleSoftwareRAID(const QString& deviceNode);

    static bool stopSoftwareRAID(const QString& deviceNode);

    static bool reassembleSoftwareRAID(const QString& deviceNode);

    static bool isRaidMember(const QString& path);

protected:
    void initPartitions() override;

    qint64 mappedSector(const QString &partitionPath, qint64 sector) const override;

private:
    static QString getDetail(const QString& path);

    static QString getRAIDConfiguration(const QString& configurationPath);
};

#endif // SOFTWARERAID_H
