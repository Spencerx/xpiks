#include "integrationtestsenvironment.h"

#include <QChar>
#include <QDebug>
#include <QDir>

#include "Common/logging.h"
#include "Helpers/filehelpers.h"

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
