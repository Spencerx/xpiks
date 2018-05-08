#ifndef SAVEWITHEMPTYTITLETEST_H
#define SAVEWITHEMPTYTITLETEST_H

#include "integrationtestbase.h"

class SaveWithEmptyTitleTest : public IntegrationTestBase
{
public:
    SaveWithEmptyTitleTest(IntegrationTestsEnvironment &environment, Commands::CommandManager *commandManager):
        IntegrationTestBase(environment, commandManager)
    {}

    // IntegrationTestBase interface
public:
    virtual QString testName();
    virtual void setup();
    virtual int doTest();
};

#endif // SAVEWITHEMPTYTITLETEST_H
