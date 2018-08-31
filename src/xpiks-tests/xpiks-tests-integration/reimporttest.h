#ifndef REIMPORTTEST_H
#define REIMPORTTEST_H

#include "integrationtestbase.h"

class ReimportTest: public IntegrationTestBase
{
public:
    ReimportTest(IntegrationTestsEnvironment &environment, XpiksTestsApp &testsApp):
        IntegrationTestBase(environment, testsApp)
    {}

    // IntegrationTestBase interface
public:
    virtual QString testName();
    virtual void setup();
    virtual int doTest();
};

#endif // REIMPORTTEST_H
