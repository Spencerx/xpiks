#ifndef WARNINGSCOMBINEDTEST_H
#define WARNINGSCOMBINEDTEST_H

#include "integrationtestbase.h"

class WarningsCombinedTest : public IntegrationTestBase
{
public:
    WarningsCombinedTest(Commands::CommandManager *commandManager):
        IntegrationTestBase(commandManager)
    {}

    // IntegrationTestBase interface
public:
    virtual QString testName();
    virtual void setup();
    virtual int doTest();
};

#endif // WARNINGSCOMBINEDTEST_H
