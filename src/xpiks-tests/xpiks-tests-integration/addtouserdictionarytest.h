#ifndef ADDTOUSERDICTIONARYTEST_H
#define ADDTOUSERDICTIONARYTEST_H

#include "integrationtestbase.h"

class AddToUserDictionaryTest:
    public IntegrationTestBase
{
public:
    AddToUserDictionaryTest(IntegrationTestsEnvironment &environment, XpiksTestsApp &testsApp):
        IntegrationTestBase(environment, testsApp)
    {}

    // IntegrationTestBase interface

public:
    virtual QString testName();
    virtual void setup();
    virtual int doTest();
};

#endif // ADDTOUSERDICTIONARYTEST_H
