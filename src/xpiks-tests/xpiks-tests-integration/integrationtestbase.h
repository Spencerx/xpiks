#ifndef INTEGRATIONTESTBASE
#define INTEGRATIONTESTBASE

#include <QString>
#include <QHash>
#include <QUrl>
#include "integrationtestsenvironment.h"

namespace Commands {
    class CommandManager;
    class MainDelegator;
}

#define VERIFY(condition, message) \
    if (!(condition)) {\
    qCritical(message);\
    return 1;\
    }

class IntegrationTestBase {
public:
    IntegrationTestBase(IntegrationTestsEnvironment &environment,
                        Commands::CommandManager *commandManager);
    virtual ~IntegrationTestBase() { }

public:
    virtual QString testName() = 0;
    virtual void setup() = 0;
    virtual int doTest() = 0;
    virtual void teardown();

protected:
    Commands::MainDelegator *xpiks();

    QUrl setupFilePathForTest(const QString &prefix);
    QUrl getFilePathForTest(const QString &prefix);
    QUrl getDirPathForTest(const QString &prefix);
    bool getIsInMemoryOnly() const { return m_Environment.getIsInMemoryOnly(); }

protected:
    Commands::CommandManager *m_CommandManager;
    IntegrationTestsEnvironment &m_Environment;
};

#endif // INTEGRATIONTESTBASE

