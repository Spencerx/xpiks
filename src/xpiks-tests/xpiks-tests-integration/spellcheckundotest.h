#ifndef SPELLCHECKUNDOTEST_H
#define SPELLCHECKUNDOTEST_H

#include "integrationtestbase.h"

class SpellCheckUndoTest : public IntegrationTestBase
{
public:
    SpellCheckUndoTest(IntegrationTestsEnvironment &environment, Commands::CommandManager *commandManager):
        IntegrationTestBase(environment, commandManager)
    {}

    // IntegrationTestBase interface
public:
    virtual QString testName();
    virtual void setup();
    virtual int doTest();
};

#endif // SPELLCHECKUNDOTEST_H
