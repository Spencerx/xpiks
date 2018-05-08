#ifndef UNDOADDDIRECTORYTEST_H
#define UNDOADDDIRECTORYTEST_H

#include "integrationtestbase.h"

class UndoAddDirectoryTest : public IntegrationTestBase
{
public:
    UndoAddDirectoryTest(IntegrationTestsEnvironment &environment, Commands::CommandManager *commandManager):
        IntegrationTestBase(environment, commandManager)
    {}

    // IntegrationTestBase interface
public:
    virtual QString testName();
    virtual void setup();
    virtual int doTest();
};

#endif // UNDOADDDIRECTORYTEST_H
