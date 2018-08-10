#ifndef AUTOIMPORTTEST_H
#define AUTOIMPORTTEST_H

#include "integrationtestbase.h"

class AutoImportTest : public IntegrationTestBase
{
public:
    AutoImportTest(IntegrationTestsEnvironment &environment, XpiksTestsApp &testsApp):
        IntegrationTestBase(environment, testsApp)
    {}

    // IntegrationTestBase interface
public:
    virtual QString testName();
    virtual void setup();
    virtual int doTest();
};

#endif // AUTOIMPORTTEST_H
