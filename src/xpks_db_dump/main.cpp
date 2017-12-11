#include <QCoreApplication>
#include <QFileInfo>
#include <iostream>
#include "../xpiks-qt/MetadataIO/metadatacache.h"
#include "../xpiks-qt/Helpers/database.h"

void myMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
    Q_UNUSED(context);
    QString logLine = qFormatLogMessage(type, context, msg);
    std::cout << logLine.toLocal8Bit().data() << std::endl;

    if (type == QtFatalMsg) {
        abort();
    }
}

int main(int argc, char *argv[]) {
    qSetMessagePattern("%{time hh:mm:ss.zzz} %{type} T#%{threadid} %{function} - %{message}");
    qInstallMessageHandler(myMessageHandler);

    QCoreApplication app(argc, argv);

    QStringList args = app.arguments();
    if (args.length() <= 1) {
        std::cout << "Missing path to Xpiks Database dir" << std::endl;
        return 1;
    }

    QString dbDirPath = args.at(1);
    if (!QFileInfo(dbDirPath).exists()) {
        std::cout << "Path to Xpiks Database dir NOT FOUND" << std::endl;
        return 1;
    }

    Helpers::DatabaseManager databaseManager;

    bool dbInitialized = databaseManager.initialize(dbDirPath);
    Q_ASSERT(dbInitialized);
    if (!dbInitialized) {
        LOG_WARNING << "Failed to initialize the DB. Xpiks will crash soon";
        return 1;
    }

    MetadataIO::MetadataCache metadataCache(&databaseManager);

    bool success = metadataCache.initialize();
    if (!success) {
        LOG_WARNING << "Failed to initialize metadata cache";
        return 1;
    }

    metadataCache.dumpToLog();

    databaseManager.prepareToFinalize();
    metadataCache.finalize();

    return 0;
}
