#include "integrationtestbase.h"

#include <QDir>

#include "testshelpers.h"
#include "xpikstestsapp.h"

IntegrationTestBase::IntegrationTestBase(IntegrationTestsEnvironment &environment,
                                         XpiksTestsApp &testsApp):
    m_TestsApp(testsApp),
    m_Environment(environment)
{ }

void IntegrationTestBase::teardown() {
    m_TestsApp.cleanup();
}

QUrl IntegrationTestBase::setupFilePathForTest(const QString &prefix, bool withVector) {
    return ::setupFilePathForTest(m_Environment.getSessionRoot(), prefix, withVector);
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
