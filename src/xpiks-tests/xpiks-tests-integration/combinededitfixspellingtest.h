#ifndef COMBINEDEDITFIXSPELLINGTEST_H
#define COMBINEDEDITFIXSPELLINGTEST_H

#include "integrationtestbase.h"

class CombinedEditFixSpellingTest : public IntegrationTestBase
{
public:
    CombinedEditFixSpellingTest(IntegrationTestsEnvironment &environment, Commands::CommandManager *commandManager):
        IntegrationTestBase(environment, commandManager)
    {}

    // IntegrationTestBase interface
public:
    virtual QString testName();
    virtual void setup();
    virtual int doTest();
};

#endif // COMBINEDEDITFIXSPELLINGTEST_H
