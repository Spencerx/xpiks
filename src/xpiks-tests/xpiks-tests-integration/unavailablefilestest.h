#ifndef UNAVAILABLEFILESTEST_H
#define UNAVAILABLEFILESTEST_H

#include "integrationtestbase.h"

class UnavailableFilesTest : public IntegrationTestBase
{
public:
    UnavailableFilesTest(IntegrationTestsEnvironment &environment, XpiksTestsApp &testsApp):
        IntegrationTestBase(environment, testsApp)
    {}

    // IntegrationTestBase interface
public:
    virtual QString testName();
    virtual void setup();
    virtual int doTest();
};

#endif // UNAVAILABLEFILESTEST_H
