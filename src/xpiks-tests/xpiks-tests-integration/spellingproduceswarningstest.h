#ifndef SPELLINGPRODUCESWARNINGSTEST_H
#define SPELLINGPRODUCESWARNINGSTEST_H

#include "integrationtestbase.h"

class SpellingProducesWarningsTest : public IntegrationTestBase
{
public:
    SpellingProducesWarningsTest(IntegrationTestsEnvironment &environment, Commands::CommandManager *commandManager):
        IntegrationTestBase(environment, commandManager)
    {}

    // IntegrationTestBase interface
public:
    virtual QString testName();
    virtual void setup();
    virtual int doTest();
};

#endif // SPELLINGPRODUCESWARNINGSTEST_H
