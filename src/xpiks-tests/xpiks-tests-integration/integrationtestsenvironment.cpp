#include "integrationtestsenvironment.h"
#include <QDateTime>
#include <QDir>
#include "../../xpiks-qt/Helpers/filehelpers.h"
#include "../../xpiks-qt/Common/logging.h"

IntegrationTestsEnvironment::IntegrationTestsEnvironment(const QStringList &appArguments):
    Common::SystemEnvironment(appArguments)
{
}

void IntegrationTestsEnvironment::ensureSystemDirectoriesExist() {
    SystemEnvironment::ensureSystemDirectoriesExist();

    m_SessionRoot = QDir::cleanPath(QDir::currentPath() + QChar('/') + getSessionTag());
    LOG_INFO << "Using" << m_SessionRoot << "as session root for tests";
    Helpers::ensureDirectoryExists(m_SessionRoot);
}
