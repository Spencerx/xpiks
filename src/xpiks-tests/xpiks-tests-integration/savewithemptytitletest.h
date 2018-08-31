#ifndef SAVEWITHEMPTYTITLETEST_H
#define SAVEWITHEMPTYTITLETEST_H

#include "integrationtestbase.h"

class SaveWithEmptyTitleTest : public IntegrationTestBase
{
public:
    SaveWithEmptyTitleTest(IntegrationTestsEnvironment &environment, XpiksTestsApp &testsApp):
        IntegrationTestBase(environment, testsApp)
    {}

    // IntegrationTestBase interface
public:
    virtual QString testName();
    virtual void setup();
    virtual int doTest();
};

#endif // SAVEWITHEMPTYTITLETEST_H
