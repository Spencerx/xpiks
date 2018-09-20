#ifndef AUTOCOMPLETEBASICTEST_H
#define AUTOCOMPLETEBASICTEST_H

#include "integrationtestbase.h"

class AutoCompleteBasicTest : public IntegrationTestBase
{
public:
    AutoCompleteBasicTest(IntegrationTestsEnvironment &environment, XpiksTestsApp &testsApp):
        IntegrationTestBase(environment, testsApp)
    {}

    // IntegrationTestBase interface
public:
    virtual QString testName();
    virtual void setup();
    virtual int doTest();
};

#endif // AUTOCOMPLETEBASICTEST_H
