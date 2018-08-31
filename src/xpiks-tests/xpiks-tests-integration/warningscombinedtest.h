#ifndef WARNINGSCOMBINEDTEST_H
#define WARNINGSCOMBINEDTEST_H

#include "integrationtestbase.h"

class WarningsCombinedTest : public IntegrationTestBase
{
public:
    WarningsCombinedTest(IntegrationTestsEnvironment &environment, XpiksTestsApp &testsApp):
        IntegrationTestBase(environment, testsApp)
    {}

    // IntegrationTestBase interface
public:
    virtual QString testName();
    virtual void setup();
    virtual int doTest();
};

#endif // WARNINGSCOMBINEDTEST_H
