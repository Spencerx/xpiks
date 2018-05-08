#ifndef FINDANDREPLACEMODELTEST_H
#define FINDANDREPLACEMODELTEST_H

#include "integrationtestbase.h"

class FindAndReplaceModelTest : public IntegrationTestBase
{
public:
    FindAndReplaceModelTest(IntegrationTestsEnvironment &environment, Commands::CommandManager *commandManager):
        IntegrationTestBase(environment, commandManager)
    {}

    // IntegrationTestBase interface
public:
    virtual QString testName();
    virtual void setup();
    virtual int doTest();
};

#endif // FINDANDREPLACEMODELTEST_H
