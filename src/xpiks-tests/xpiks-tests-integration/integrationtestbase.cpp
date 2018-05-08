#include "integrationtestbase.h"
#include <QDebug>
#include <QDir>
#include "../../xpiks-qt/Commands/commandmanager.h"
#include "../../xpiks-qt/Models/settingsmodel.h"
#include "testshelpers.h"

void ensureDirectoryExistsForFile(const QString &filepath) {
    QFileInfo fi(filepath);

    if (fi.exists()) { return; }

    QDir filesDir = fi.absoluteDir();
    if (!filesDir.exists()) {
        if (!filesDir.mkpath(".")) {
            qWarning() << "Failed to create a path:" << filesDir.path();
        }
    }
}

bool copyFile(const QString &from, const QString &to) {
    bool success = false;

    QFile destination(to);
    if (destination.exists()) {
        if (!destination.remove()) {
            return success;
        }
    }

    QFile source(from);
    if (source.exists()) {
        success = source.copy(to);
    }

    return success;
}

IntegrationTestBase::IntegrationTestBase(IntegrationTestsEnvironment &environment,
                                         Commands::CommandManager *commandManager):
    m_CommandManager(commandManager),
    m_Environment(environment)
{ }

void IntegrationTestBase::teardown() {
    auto *settingsModel = m_CommandManager->getSettingsModel();
    const QString exiftoolPath = settingsModel->getExifToolPath();
    {
        m_CommandManager->cleanup();
    }
    settingsModel->setExifToolPath(exiftoolPath);
}

Commands::MainDelegator *IntegrationTestBase::xpiks() { return m_CommandManager->getDelegator(); }

QUrl IntegrationTestBase::setupFilePathForTest(const QString &prefix) {
    QString fullPath;
    if (tryFindFullPathForTests(prefix, fullPath)) {
        qInfo() << "Found file at" << fullPath;
        QString sessionPath = QDir::cleanPath(m_Environment.getSessionRoot() + QChar('/') + prefix);
        ensureDirectoryExistsForFile(sessionPath);
        if (copyFile(fullPath, sessionPath)) {
            qInfo() << "Copied to" << sessionPath;
            return QUrl::fromLocalFile(sessionPath);
        } else {
            return QUrl::fromLocalFile(fullPath);
        }
    }

    return QUrl::fromLocalFile(prefix);
}

QUrl IntegrationTestBase::getFilePathForTest(const QString &prefix) {
    return QUrl::fromLocalFile(findFullPathForTests(prefix));
}

QUrl IntegrationTestBase::getDirPathForTest(const QString &prefix) {
    QString path = prefix;
    if (path.startsWith('/')) { path.remove(0, 1); }
    QDir dir(path);
    int tries = 6;
    while (tries--) {
        if (!dir.exists()) {
            path = "../" + prefix;
            dir.setPath(path);
        } else {
            return QUrl::fromLocalFile(dir.absolutePath());
        }
    }

    return QUrl::fromLocalFile(QDir(prefix).absolutePath());
}
