#ifndef LOADPLUGINBASICTEST_H
#define LOADPLUGINBASICTEST_H

#include "integrationtestbase.h"

class LoadPluginBasicTest : public IntegrationTestBase
{
public:
    LoadPluginBasicTest(Commands::CommandManager *commandManager):
        IntegrationTestBase(commandManager)
    {}

    // IntegrationTestBase interface
public:
    virtual QString testName();
    virtual void setup();
    virtual int doTest();
};

#endif // LOADPLUGINBASICTEST_H
