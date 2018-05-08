#ifndef AUTOIMPORTTEST_H
#define AUTOIMPORTTEST_H

#include "integrationtestbase.h"

class AutoImportTest : public IntegrationTestBase
{
public:
    AutoImportTest(IntegrationTestsEnvironment &environment, Commands::CommandManager *commandManager):
        IntegrationTestBase(environment, commandManager)
    {}

    // IntegrationTestBase interface
public:
    virtual QString testName();
    virtual void setup();
    virtual int doTest();
};

#endif // AUTOIMPORTTEST_H
