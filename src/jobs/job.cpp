/*************************************************************************
 *  Copyright (C) 2008, 2009, 2010 by Volker Lanz <vl@fidra.de>          *
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

#include "jobs/job.h"

#include "core/device.h"
#include "core/copysource.h"
#include "core/copytarget.h"
#include "core/copysourcedevice.h"
#include "core/copytargetdevice.h"

#include "util/report.h"

#include <QIcon>
#include <QTime>

#include <KLocalizedString>

Job::Job() :
    m_Status(Pending)
{
}

bool Job::copyBlocks(Report& report, CopyTarget& target, CopySource& source)
{
    bool rval = true;
    const qint64 blockSize = 10 * 1024 * 1024; // number of bytes per block to copy
    const qint64 blocksToCopy = source.length() / blockSize;

    qint64 readOffset = source.firstByte();
    qint64 writeOffset = target.firstByte();
    qint32 copyDirection = 1;

    if (target.firstByte() > source.firstByte()) {
        readOffset = source.firstByte() + source.length() - blockSize;
        writeOffset = target.firstByte() + source.length() - blockSize;
        copyDirection = -1;
    }

    report.line() << xi18nc("@info:progress", "Copying %1 blocks (%2 bytes) from %3 to %4, direction: %5.", blocksToCopy, source.length(), readOffset, writeOffset, copyDirection == 1 ? i18nc("direction: left", "left") : i18nc("direction: right", "right"));

    qint64 blocksCopied = 0;

    QByteArray buffer;
    int percent = 0;
    QTime t;
    t.start();
    while (blocksCopied < blocksToCopy) {
        if (!(rval = source.readData(buffer, readOffset + blockSize * blocksCopied * copyDirection, blockSize)))
            break;

        if (!(rval = target.writeData(buffer, writeOffset + blockSize * blocksCopied * copyDirection)))
            break;

        if (++blocksCopied * 100 / blocksToCopy != percent) {
            percent = blocksCopied * 100 / blocksToCopy;

            if (percent % 5 == 0 && t.elapsed() > 1000) {
                const qint64 mibsPerSec = (blocksCopied * blockSize / 1024 / 1024) / (t.elapsed() / 1000);
                const qint64 estSecsLeft = (100 - percent) * t.elapsed() / percent / 1000;
                report.line() << xi18nc("@info:progress", "Copying %1 MiB/second, estimated time left: %2", mibsPerSec, QTime(0, 0).addSecs(estSecsLeft).toString());
            }
            emit progress(percent);
        }
    }
    const qint64 lastBlock = source.length() % blockSize;

    // copy the remainder
    if (rval && lastBlock > 0) {
        Q_ASSERT(lastBlock < blockSize);

        const qint64 lastBlockReadOffset = copyDirection > 0 ? readOffset + blockSize * blocksCopied : source.firstByte();
        const qint64 lastBlockWriteOffset = copyDirection > 0 ? writeOffset + blockSize * blocksCopied : target.firstByte();

        report.line() << xi18nc("@info:progress", "Copying remainder of block size %1 from %2 to %3.", lastBlock, lastBlockReadOffset, lastBlockWriteOffset);

        rval = source.readData(buffer, lastBlockReadOffset, lastBlock);

        if (rval)
            rval = target.writeData(buffer, lastBlockWriteOffset);

        if (rval)
            emit progress(100);
    }


    report.line() << xi18ncp("@info:progress argument 2 is a string such as 7 bytes (localized accordingly)", "Copying 1 block (%2) finished.", "Copying %1 blocks (%2) finished.", blocksCopied, i18np("1 byte", "%1 bytes", target.bytesWritten()));

    return rval;
}

bool Job::rollbackCopyBlocks(Report& report, CopyTarget& origTarget, CopySource& origSource)
{
    if (!origSource.overlaps(origTarget)) {
        report.line() << xi18nc("@info:progress", "Source and target for copying do not overlap: Rollback is not required.");
        return true;
    }

    try {
        CopySourceDevice& csd = dynamic_cast<CopySourceDevice&>(origSource);
        CopyTargetDevice& ctd = dynamic_cast<CopyTargetDevice&>(origTarget);

        // default: use values as if we were copying from front to back.
        qint64 undoSourceFirstByte = origTarget.firstByte();
        qint64 undoSourceLastByte = origTarget.firstByte() + origTarget.bytesWritten() - 1;

        qint64 undoTargetFirstByte = origSource.firstByte();
        qint64 undoTargetLastByte = origSource.firstByte() + origTarget.bytesWritten() - 1;

        if (origTarget.firstByte() > origSource.firstByte()) {
            // we were copying from back to front
            undoSourceFirstByte = origTarget.firstByte() + origSource.length() - origTarget.bytesWritten();
            undoSourceLastByte = origTarget.firstByte() + origSource.length() - 1;

            undoTargetFirstByte = origSource.lastByte() - origTarget.bytesWritten() + 1;
            undoTargetLastByte = origSource.lastByte();
        }

        report.line() << xi18nc("@info:progress", "Rollback from: First byte: %1, last byte: %2.", undoSourceFirstByte, undoSourceLastByte);
        report.line() << xi18nc("@info:progress", "Rollback to: First byte: %1, last byte: %2.", undoTargetFirstByte, undoTargetLastByte);

        CopySourceDevice undoSource(ctd.device(), undoSourceFirstByte, undoSourceLastByte);
        if (!undoSource.open()) {
            report.line() << xi18nc("@info:progress", "Could not open device <filename>%1</filename> to rollback copying.", ctd.device().deviceNode());
            return false;
        }

        CopyTargetDevice undoTarget(csd.device(), undoTargetFirstByte, undoTargetLastByte);
        if (!undoTarget.open()) {
            report.line() << xi18nc("@info:progress", "Could not open device <filename>%1</filename> to rollback copying.", csd.device().deviceNode());
            return false;
        }

        return copyBlocks(report, undoTarget, undoSource);
    } catch (...) {
        report.line() << xi18nc("@info:progress", "Rollback failed: Source or target are not devices.");
    }

    return false;
}

void Job::emitProgress(int i)
{
    emit progress(i);
}

Report* Job::jobStarted(Report& parent)
{
    emit started();

    return parent.newChild(xi18nc("@info:progress", "Job: %1", description()));
}

void Job::jobFinished(Report& report, bool b)
{
    setStatus(b ? Success : Error);
    emit progress(numSteps());
    emit finished();

    report.setStatus(xi18nc("@info:progress job status (error, warning, ...)", "%1: %2", description(), statusText()));
}

/** @return the Job's current status icon */
QString Job::statusIcon() const
{
    static const QString icons[] = {
        QStringLiteral("dialog-information"),
        QStringLiteral("dialog-ok"),
        QStringLiteral("dialog-error")
    };

    Q_ASSERT(status() >= 0 && static_cast<quint32>(status()) < sizeof(icons) / sizeof(icons[0]));

    if (status() < 0 || static_cast<quint32>(status()) >= sizeof(icons) / sizeof(icons[0]))
        return QString();

    return icons[status()];
}

/** @return the Job's current status text */
QString Job::statusText() const
{
    static const QString s[] = {
        xi18nc("@info:progress job", "Pending"),
        xi18nc("@info:progress job", "Success"),
        xi18nc("@info:progress job", "Error")
    };

    Q_ASSERT(status() >= 0 && static_cast<quint32>(status()) < sizeof(s) / sizeof(s[0]));

    if (status() < 0 || static_cast<quint32>(status()) >= sizeof(s) / sizeof(s[0]))
        return QString();

    return s[status()];
}
