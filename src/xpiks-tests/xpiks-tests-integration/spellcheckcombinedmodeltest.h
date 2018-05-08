#ifndef SPELLCHECKCOMBINEDMODELTEST_H
#define SPELLCHECKCOMBINEDMODELTEST_H

#include "integrationtestbase.h"

class SpellCheckCombinedModelTest : public IntegrationTestBase
{
public:
    SpellCheckCombinedModelTest(IntegrationTestsEnvironment &environment, Commands::CommandManager *commandManager):
        IntegrationTestBase(environment, commandManager)
    {}

    // IntegrationTestBase interface
public:
    virtual QString testName();
    virtual void setup();
    virtual int doTest();
};

#endif // SPELLCHECKCOMBINEDMODELTEST_H
