#ifndef REIMPORTTEST_H
#define REIMPORTTEST_H

#include "integrationtestbase.h"

class ReimportTest: public IntegrationTestBase
{
public:
    ReimportTest(IntegrationTestsEnvironment &environment, Commands::CommandManager *commandManager):
        IntegrationTestBase(environment, commandManager)
    {}

    // IntegrationTestBase interface
public:
    virtual QString testName();
    virtual void setup();
    virtual int doTest();
};

#endif // REIMPORTTEST_H
