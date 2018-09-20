#ifndef ZIPARTWORKSTEST_H
#define ZIPARTWORKSTEST_H

#include "integrationtestbase.h"

class ZipArtworksTest : public IntegrationTestBase
{
public:
    ZipArtworksTest(IntegrationTestsEnvironment &environment, XpiksTestsApp &testsApp):
        IntegrationTestBase(environment, testsApp)
    {}

    // IntegrationTestBase interface
public:
    virtual QString testName();
    virtual void setup();
    virtual int doTest();
};

#endif // ZIPARTWORKSTEST_H
