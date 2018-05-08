#ifndef ADDFILESBASICTEST_H
#define ADDFILESBASICTEST_H

#include "integrationtestbase.h"

class AddFilesBasicTest : public IntegrationTestBase
{
public:
    AddFilesBasicTest(IntegrationTestsEnvironment &environment, Commands::CommandManager *commandManager):
        IntegrationTestBase(environment, commandManager)
    {}

    // IntegrationTestBase interface
public:
    virtual QString testName();
    virtual void setup();
    virtual int doTest();
};

#endif // ADDFILESBASICTEST_H
