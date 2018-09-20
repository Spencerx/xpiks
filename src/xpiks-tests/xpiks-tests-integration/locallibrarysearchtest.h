#ifndef LOCALLIBRARYSEARCHTEST_H
#define LOCALLIBRARYSEARCHTEST_H

#include "integrationtestbase.h"

class LocalLibrarySearchTest : public IntegrationTestBase
{
public:
    LocalLibrarySearchTest(IntegrationTestsEnvironment &environment, XpiksTestsApp &testsApp):
        IntegrationTestBase(environment, testsApp)
    {}

    // IntegrationTestBase interface
public:
    virtual QString testName();
    virtual void setup();
    virtual int doTest();
};

#endif // LOCALLIBRARYSEARCHTEST_H
