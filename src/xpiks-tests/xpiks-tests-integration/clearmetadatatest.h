#ifndef CLEARMETADATATEST_H
#define CLEARMETADATATEST_H

#include "integrationtestbase.h"

class ClearMetadataTest : public IntegrationTestBase
{
public:
    ClearMetadataTest(IntegrationTestsEnvironment &environment, XpiksTestsApp &testsApp):
        IntegrationTestBase(environment, testsApp)
    {}

    // IntegrationTestBase interface
public:
    virtual QString testName();
    virtual void setup();
    virtual int doTest();
};

#endif // CLEARMETADATATEST_H
