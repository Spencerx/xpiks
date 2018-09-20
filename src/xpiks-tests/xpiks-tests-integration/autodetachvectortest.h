#ifndef AUTODETACHVECTORTEST_H
#define AUTODETACHVECTORTEST_H

#include "integrationtestbase.h"

class AutoDetachVectorTest : public IntegrationTestBase
{
public:
    AutoDetachVectorTest(IntegrationTestsEnvironment &environment, XpiksTestsApp &testsApp):
        IntegrationTestBase(environment, testsApp)
    {}

    // IntegrationTestBase interface
public:
    virtual QString testName();
    virtual void setup();
    virtual int doTest();
};

#endif // AUTODETACHVECTORTEST_H
