#ifndef WARNINGSCOMBINEDTEST_H
#define WARNINGSCOMBINEDTEST_H

#include "integrationtestbase.h"

class WarningsCombinedTest : public IntegrationTestBase
{
public:
    WarningsCombinedTest(IntegrationTestsEnvironment &environment, Commands::CommandManager *commandManager):
        IntegrationTestBase(environment, commandManager)
    {}

    // IntegrationTestBase interface
public:
    virtual QString testName();
    virtual void setup();
    virtual int doTest();
};

#endif // WARNINGSCOMBINEDTEST_H
