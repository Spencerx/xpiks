#ifndef DUPLICATESEARCH_TEST_H
#define DUPLICATESEARCH_TEST_H

#include "integrationtestbase.h"

class DuplicateSearchTest: public IntegrationTestBase
{
public:
    DuplicateSearchTest(IntegrationTestsEnvironment &environment, XpiksTestsApp &testsApp):
        IntegrationTestBase(environment, testsApp)
    {}

    // IntegrationTestBase interface
public:
    virtual QString testName();
    virtual void setup();
    virtual int doTest();
};

#endif // DUPLICATESEARCH_TEST_H
