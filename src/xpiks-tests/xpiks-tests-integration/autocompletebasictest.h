#ifndef AUTOCOMPLETEBASICTEST_H
#define AUTOCOMPLETEBASICTEST_H

#include "integrationtestbase.h"

class AutoCompleteBasicTest : public IntegrationTestBase
{
public:
    AutoCompleteBasicTest(IntegrationTestsEnvironment &environment, Commands::CommandManager *commandManager):
        IntegrationTestBase(environment, commandManager)
    {}

    // IntegrationTestBase interface
public:
    virtual QString testName();
    virtual void setup();
    virtual int doTest();
};

#endif // AUTOCOMPLETEBASICTEST_H
