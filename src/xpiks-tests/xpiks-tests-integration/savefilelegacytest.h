#ifndef SAVEDIRECTEXPORTTEST_H
#define SAVEDIRECTEXPORTTEST_H

#include "integrationtestbase.h"

class SaveFileLegacyTest : public IntegrationTestBase
{
public:
    SaveFileLegacyTest(IntegrationTestsEnvironment &environment, Commands::CommandManager *commandManager):
        IntegrationTestBase(environment, commandManager)
    {}

    // IntegrationTestBase interface
public:
    virtual QString testName();
    virtual void setup();
    virtual int doTest();
};

#endif // SAVEDIRECTEXPORTTEST_H
