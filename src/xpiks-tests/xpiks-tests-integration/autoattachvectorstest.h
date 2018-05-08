#ifndef AUTOATTACHVECTORSTEST_H
#define AUTOATTACHVECTORSTEST_H

#include "integrationtestbase.h"

class AutoAttachVectorsTest : public IntegrationTestBase
{
public:
    AutoAttachVectorsTest(IntegrationTestsEnvironment &environment, Commands::CommandManager *commandManager):
        IntegrationTestBase(environment, commandManager)
    {}

    // IntegrationTestBase interface
public:
    virtual QString testName();
    virtual void setup();
    virtual int doTest();
};

#endif // AUTOATTACHVECTORSTEST_H
