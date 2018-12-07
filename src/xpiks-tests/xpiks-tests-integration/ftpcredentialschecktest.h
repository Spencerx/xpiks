#ifndef FTPCREDENTIALSCHECKTEST_H
#define FTPCREDENTIALSCHECKTEST_H

#include "integrationtestbase.h"

class FtpCredentialsCheckTest : public IntegrationTestBase
{
public:
    FtpCredentialsCheckTest(IntegrationTestsEnvironment &environment, XpiksTestsApp &testsApp):
        IntegrationTestBase(environment, testsApp)
    {}

    // IntegrationTestBase interface
public:
    virtual QString testName();
    virtual void setup();
    virtual int doTest();
};

#endif // FTPCREDENTIALSCHECKTEST_H
