#ifndef PRESETTESTS_H
#define PRESETTESTS_H

#include "integrationtestbase.h"

class PresetsTest: public IntegrationTestBase
{
public:
    PresetsTest(IntegrationTestsEnvironment &environment, Commands::CommandManager *commandManager):
        IntegrationTestBase(environment, commandManager)
    {}

    // IntegrationTestBase interface
public:
    virtual QString testName();
    virtual void setup();
    virtual int doTest();
};

#endif // PRESETTESTS_H
