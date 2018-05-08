#ifndef REMOVEFROMUSERDICTIONARYTEST_H
#define REMOVEFROMUSERDICTIONARYTEST_H

#include "integrationtestbase.h"

class RemoveFromUserDictionaryTest:
    public IntegrationTestBase
{
public:
    RemoveFromUserDictionaryTest(IntegrationTestsEnvironment &environment, Commands::CommandManager *commandManager):
        IntegrationTestBase(environment, commandManager)
    {}

    // IntegrationTestBase interface

public:
    virtual QString testName();
    virtual void setup();
    virtual int doTest();
};

#endif // REMOVEFROMUSERDICTIONARYTEST_H
