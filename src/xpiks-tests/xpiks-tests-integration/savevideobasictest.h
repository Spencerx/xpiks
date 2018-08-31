#ifndef SAVEVIDEOBASICTEST_H
#define SAVEVIDEOBASICTEST_H

#include "integrationtestbase.h"

class SaveVideoBasicTest : public IntegrationTestBase
{
public:
    SaveVideoBasicTest(IntegrationTestsEnvironment &environment, XpiksTestsApp &testsApp):
        IntegrationTestBase(environment, testsApp)
    {}

    // IntegrationTestBase interface
public:
    virtual QString testName();
    virtual void setup();
    virtual int doTest();
};

#endif // SAVEVIDEOBASICTEST_H
