/*************************************************************************
 *  Copyright (C) 2008 by Volker Lanz <vl@fidra.de>                      *
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

#if !defined(KPMCORE_EXTERNALCOMMAND_H)

#define KPMCORE_EXTERNALCOMMAND_H

#include "util/libpartitionmanagerexport.h"
#include "core/copysourcedevice.h"
#include "core/copytargetfile.h"

#include <KJob>

#include <QDebug>
#include <QProcess>
#include <QString>
#include <QStringList>
#include <QtGlobal>
#include <QVariant>

class Report;

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
    explicit ExternalCommand(CopySource& source, CopyTarget& target, QProcess::ProcessChannelMode processChannelMode = QProcess::MergedChannels);

public:
    bool copyBlocks();
    void setCommand(const QString& cmd) { m_Command = cmd; } /**< @param cmd the command to run */
    const QString& command() const { return m_Command; } /**< @return the command to run */

    void addArg(const QString& s) { m_Args << s; } /**< @param s the argument to add */
    const QStringList& args() const { return m_Args; } /**< @return the arguments */
    void setArgs(const QStringList& args) { m_Args = args; } /**< @param args the new arguments */
    bool write(const QByteArray& input); /**< @param input the input for the program */

    bool startCopyBlocks();
    bool start(int timeout = 30000);
    bool waitFor(int timeout = 30000);
    bool run(int timeout = 30000);

    int exitCode() const {
        return m_ExitCode;    /**< @return the exit code */
    }

    const QString output() const {
        return QString::fromLocal8Bit(m_Output);    /**< @return the command output */
    }

    const QByteArray& rawOutput() const {
        return m_Output;    /**< @return the command output */
    }

    Report* report() {
        return m_Report;    /**< @return pointer to the Report or nullptr */
    }

    void emitReport(const QVariantMap& report) { emit reportSignal(report); }

Q_SIGNALS:
    void progress(int);
    void finished();
    void reportSignal(const QVariantMap&);

public Q_SLOTS:
    void emitProgress(KJob*, unsigned long percent) { emit progress(percent); };

protected:
    void setExitCode(int i) {
        m_ExitCode = i;
    }
    void setup(const QProcess::ProcessChannelMode processChannelMode);

    void onFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onReadOutput();

private:
    QVariantMap arguments;

    Report *m_Report;
    QString m_Command;
    QStringList m_Args;
    int m_ExitCode;
    QByteArray m_Output;
    QByteArray m_Input;
    CopySource *m_Source;
    CopyTarget *m_Target;
};

#endif
