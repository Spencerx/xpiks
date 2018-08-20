#include "integrationtestbase.h"
#include <QDebug>
#include <QDir>
#include <Helpers/filehelpers.h>
#include "xpikstestsapp.h"
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
                                         XpiksTestsApp &testsApp):
    m_TestsApp(testsApp),
    m_Environment(environment)
{ }

void IntegrationTestBase::teardown() {
    m_TestsApp.cleanup();
}

QUrl IntegrationTestBase::setupFilePathForTest(const QString &prefix, bool withVector) {
    QString fullPath;
    if (tryFindFullPathForTests(prefix, fullPath)) {
        qInfo() << "Found artwork at" << fullPath;
        QString sessionPath = QDir::cleanPath(m_Environment.getSessionRoot() + QChar('/') + prefix);
        ensureDirectoryExistsForFile(sessionPath);
        if (copyFile(fullPath, sessionPath)) {
            qInfo() << "Copied artwork to" << sessionPath;

            if (withVector) {
                QStringList vectors = Helpers::convertToVectorFilenames(fullPath);
                QStringList vectorPrefixes = Helpers::convertToVectorFilenames(prefix);
                Q_ASSERT(vectors.size() == vectorPrefixes.size());
                for (int i = 0; i < vectors.size(); i++) {
                    if (QFileInfo(vectors[i]).exists()) {
                        qInfo() << "Found vector at" << vectors[i];
                        QString vectorSessionPath = QDir::cleanPath(m_Environment.getSessionRoot() + QChar('/') + vectorPrefixes[i]);
                        copyFile(vectors[i], vectorSessionPath);
                        qInfo() << "Copied vector to" << vectorSessionPath;
                        break;
                    }
                }
            }

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
