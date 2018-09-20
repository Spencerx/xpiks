#ifndef FAILEDUPLOADSTEST_H
#define FAILEDUPLOADSTEST_H

#include "integrationtestbase.h"

class FailedUploadsTest : public IntegrationTestBase
{
public:
    FailedUploadsTest(IntegrationTestsEnvironment &environment, XpiksTestsApp &testsApp):
        IntegrationTestBase(environment, testsApp)
    {}

    // IntegrationTestBase interface
public:
    virtual QString testName();
    virtual void setup();
    virtual int doTest();
};

#endif // FAILEDUPLOADSTEST_H
