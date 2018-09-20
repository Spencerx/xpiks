#ifndef INTEGRATIONTESTBASE
#define INTEGRATIONTESTBASE

#include <QString>
#include <QHash>
#include <QUrl>
#include "integrationtestsenvironment.h"

class XpiksTestsApp;

#define VERIFY(condition, message) \
    if (!(condition)) {\
    qCritical(message);\
    return 1;\
    }

class IntegrationTestBase {
public:
    IntegrationTestBase(IntegrationTestsEnvironment &environment,
                        XpiksTestsApp &testsApp);
    virtual ~IntegrationTestBase() { }

public:
    virtual QString testName() = 0;
    virtual void setup() = 0;
    virtual int doTest() = 0;
    virtual void teardown();

protected:
    QUrl setupFilePathForTest(const QString &prefix, bool withVector=false);
    QUrl getFilePathForTest(const QString &prefix);
    QUrl getDirPathForTest(const QString &prefix);
    bool getIsInMemoryOnly() const { return m_Environment.getIsInMemoryOnly(); }

protected:
    XpiksTestsApp &m_TestsApp;
    IntegrationTestsEnvironment &m_Environment;
};

#endif // INTEGRATIONTESTBASE

