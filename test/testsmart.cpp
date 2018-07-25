#include "helpers.h"

#include "util/externalcommand.h"
#include "backend/corebackend.h"
#include "backend/corebackendmanager.h"
#include "core/smartstatus.h"
#include "core/smartparser.h"

#include <QCoreApplication>
#include <QDebug>

static QString getDefaultDevicePath();
static bool testSmartStatus();
static bool testSmartParser();

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);

    KPMCoreInitializer i;

    if (argc == 2)
        i = KPMCoreInitializer(argv[1]);

    if (!i.isValid())
        return 1;

    CoreBackend *backend = CoreBackendManager::self()->backend();

    if (!backend)
    {
        qWarning() << "Couldn't get backend.";
        return 1;
    }

    if (!testSmartStatus() || !testSmartParser())
        return 1;

    return app.exec();
}

static QString getDefaultDevicePath()
{
    // Getting default home partition using 'df -P /home | awk 'END{print $1}'' command
    ExternalCommand command(QStringLiteral("df"), { QStringLiteral("-P"), QStringLiteral("/home"), QStringLiteral("|"),
                                                    QStringLiteral("awk"), QStringLiteral("\'END{print $1}\'") });

    if (command.run() && command.exitCode() == 0) {
        QString output = command.output();
        return output;
    }

    return QString();
}

static bool testSmartStatus()
{
    QString devicePath = getDefaultDevicePath();

    SmartStatus smart(devicePath);

    if (smart.devicePath() != devicePath)
        return false;

    if (!smart.status())
        return false;

    if (smart.modelName() == QString())
        return false;

    if (smart.firmware() == QString())
        return false;

    if (smart.serial() == QString())
        return false;

    if (smart.selfTestStatus() != SmartStatus::SelfTestStatus::Success)
        return false;

    if (!smart.isValid())
        return false;

    return true;
}

static bool testSmartParser()
{
    QString devicePath = getDefaultDevicePath();

    SmartParser parser(devicePath);

    if (!parser.init())
        return false;

    if (parser.devicePath() != devicePath)
        return false;

    if (!parser.diskInformation())
        return false;

    return true;
}
