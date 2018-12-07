#ifndef FTPUPLOADTEST_H
#define FTPUPLOADTEST_H

#include "integrationtestbase.h"

class FtpUploadTest : public IntegrationTestBase
{
public:
    FtpUploadTest(IntegrationTestsEnvironment &environment, XpiksTestsApp &testsApp):
        IntegrationTestBase(environment, testsApp)
    {}

    // IntegrationTestBase interface
public:
    virtual QString testName();
    virtual void setup();
    virtual int doTest();
};

#endif // FTPUPLOADTEST_H
