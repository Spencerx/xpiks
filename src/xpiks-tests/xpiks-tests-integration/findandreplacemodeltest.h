#ifndef FINDANDREPLACEMODELTEST_H
#define FINDANDREPLACEMODELTEST_H

#include "integrationtestbase.h"

class FindAndReplaceModelTest : public IntegrationTestBase
{
public:
    FindAndReplaceModelTest(IntegrationTestsEnvironment &environment, XpiksTestsApp &testsApp):
        IntegrationTestBase(environment, testsApp)
    {}

    // IntegrationTestBase interface
public:
    virtual QString testName();
    virtual void setup();
    virtual int doTest();
};

#endif // FINDANDREPLACEMODELTEST_H
