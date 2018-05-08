#ifndef UNDOADDWITHVECTORSTEST_H
#define UNDOADDWITHVECTORSTEST_H

#include "integrationtestbase.h"

class UndoAddWithVectorsTest : public IntegrationTestBase
{
public:
    UndoAddWithVectorsTest(IntegrationTestsEnvironment &environment, Commands::CommandManager *commandManager):
        IntegrationTestBase(environment, commandManager)
    {}

    // IntegrationTestBase interface
public:
    virtual QString testName();
    virtual void setup();
    virtual int doTest();
};

#endif // UNDOADDWITHVECTORSTEST_H
