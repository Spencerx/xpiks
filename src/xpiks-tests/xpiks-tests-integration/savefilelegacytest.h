#ifndef SAVEDIRECTEXPORTTEST_H
#define SAVEDIRECTEXPORTTEST_H

#include "integrationtestbase.h"

class SaveFileLegacyTest : public IntegrationTestBase
{
public:
    SaveFileLegacyTest(IntegrationTestsEnvironment &environment, XpiksTestsApp &testsApp):
        IntegrationTestBase(environment, testsApp)
    {}

    // IntegrationTestBase interface
public:
    virtual QString testName();
    virtual void setup();
    virtual int doTest();
};

#endif // SAVEDIRECTEXPORTTEST_H
