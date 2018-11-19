#ifndef REMOVEDIRSAVESSESSIONTEST_H
#define REMOVEDIRSAVESSESSIONTEST_H

#include "integrationtestbase.h"

class RemoveDirSavesSessionTest:
        public IntegrationTestBase
{
public:
    RemoveDirSavesSessionTest(IntegrationTestsEnvironment &environment, XpiksTestsApp &testsApp):
        IntegrationTestBase(environment, testsApp)
    {}

    // IntegrationTestBase interface

public:
    virtual QString testName();
    virtual void setup();
    virtual int doTest();
};

#endif // REMOVEDIRSAVESSESSIONTEST_H
