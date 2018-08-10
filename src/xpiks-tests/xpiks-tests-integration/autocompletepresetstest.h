#ifndef AUTOCOMPLETEPRESETSTEST_H
#define AUTOCOMPLETEPRESETSTEST_H

#include "integrationtestbase.h"

class AutoCompletePresetsTest : public IntegrationTestBase
{
public:
    AutoCompletePresetsTest(IntegrationTestsEnvironment &environment, XpiksTestsApp &testsApp):
        IntegrationTestBase(environment, testsApp)
    {}

    // IntegrationTestBase interface
public:
    virtual QString testName() override;
    virtual void setup() override;
    virtual void teardown() override;
    virtual int doTest() override;
};

#endif // AUTOCOMPLETEPRESETSTEST_H
