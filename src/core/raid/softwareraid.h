/*
    SPDX-FileCopyrightText: 2018 Caio Jordão Carvalho <caiojcarvalho@gmail.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KPMCORE_SOFTWARERAID_H
#define KPMCORE_SOFTWARERAID_H

#include "core/volumemanagerdevice.h"
#include "util/libpartitionmanagerexport.h"
#include "util/report.h"

class LIBKPMCORE_EXPORT SoftwareRAID : public VolumeManagerDevice
{
    Q_DISABLE_COPY(SoftwareRAID)

    friend class VolumeManagerDevice;

public:
    enum class Status {
        Active,
        Inactive,
        Resync,
        Recovery,
    };

    explicit SoftwareRAID(const QString& name,
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
    static void scanSoftwareRAID(QList<Device*>& devices);

    static QString getDetail(const QString& path);

    static QString getRAIDConfiguration(const QString& configurationPath);
};

#endif // SOFTWARERAID_H
