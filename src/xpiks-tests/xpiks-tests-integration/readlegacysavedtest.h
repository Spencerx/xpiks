#ifndef READLEGACYSAVEDTEST_H
#define READLEGACYSAVEDTEST_H

#include "integrationtestbase.h"

class ReadLegacySavedTest: public IntegrationTestBase
{
public:
    ReadLegacySavedTest(IntegrationTestsEnvironment &environment, Commands::CommandManager *commandManager):
        IntegrationTestBase(environment, commandManager)
    {}

    // IntegrationTestBase interface
public:
    virtual QString testName();
    virtual void setup();
    virtual int doTest();
};

#endif // READLEGACYSAVEDTEST_H
