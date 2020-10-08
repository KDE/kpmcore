/*************************************************************************
 *  Copyright (C) 2008 by Volker Lanz <vl@fidra.de>                      *
 *  Copyright (C) 2016-2018 by Andrius Štikonas <andrius@stikonas.eu>    *
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

#ifndef KPMCORE_EXTERNALCOMMAND_H
#define KPMCORE_EXTERNALCOMMAND_H

#include "util/libpartitionmanagerexport.h"

#include <QDebug>
#include <QProcess>
#include <QString>
#include <QStringList>
#include <QtGlobal>
#include <QThread>
#include <QVariant>

#include <memory>

namespace KAuth { class ExecuteJob; }

class KJob;
class Report;
class CopySource;
class CopyTarget;
class QDBusInterface;

struct ExternalCommandPrivate;

/** An external command.

    Runs an external command as a child process.

    @author Volker Lanz <vl@fidra.de>
    @author Andrius Štikonas <andrius@stikonas.eu>
*/
class LIBKPMCORE_EXPORT ExternalCommand : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(ExternalCommand)

public:
    explicit ExternalCommand(const QString& cmd = QString(), const QStringList& args = QStringList(), const QProcess::ProcessChannelMode processChannelMode = QProcess::MergedChannels);
    explicit ExternalCommand(Report& report, const QString& cmd = QString(), const QStringList& args = QStringList(), const QProcess::ProcessChannelMode processChannelMode = QProcess::MergedChannels);

    ~ExternalCommand();

public:
    bool copyBlocks(const CopySource& source, CopyTarget& target);
    bool writeData(Report& commandReport, const QByteArray& buffer, const QString& deviceNode, const quint64 firstByte); // same as copyBlocks but from QByteArray

    /**< @param cmd the command to run */
    void setCommand(const QString& cmd);
     /**< @return the command to run */
    const QString& command() const;

    /**< @return the arguments */
    const QStringList& args() const;

    /**< @param s the argument to add */
    void addArg(const QString& s);
    /**< @param args the new arguments */
    void setArgs(const QStringList& args);

    bool write(const QByteArray& input); /**< @param input the input for the program */

    bool startCopyBlocks();
    bool start(int timeout = 30000);
    bool run(int timeout = 30000);

    /**< @return the exit code */
    int exitCode() const;

    /**< @return the command output */
    const QString output() const;
    /**< @return the command output */
    const QByteArray& rawOutput() const;

    /**< @return pointer to the Report or nullptr */
    Report* report();

    /**< stop ExternalCommand Helper */
    static void stopHelper();


    /**< Sets a parent widget for the authentication dialog.
     * @param p parent widget
     */
    static void setParentWidget(QWidget *p) {
        parent = p;
    }

Q_SIGNALS:
    void progress(int);
    void reportSignal(const QString&);

private:
    void setExitCode(int i);
    void onReadOutput();

private:
    std::unique_ptr<ExternalCommandPrivate> d;

    // KAuth
    static KAuth::ExecuteJob *m_job;
    static QWidget *parent;
};

#endif
