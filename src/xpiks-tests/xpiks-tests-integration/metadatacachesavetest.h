#ifndef METADATACACHESAVETEST_H
#define METADATACACHESAVETEST_H

#include "integrationtestbase.h"

class MetadataCacheSaveTest : public IntegrationTestBase
{
public:
    MetadataCacheSaveTest(IntegrationTestsEnvironment &environment, Commands::CommandManager *commandManager):
        IntegrationTestBase(environment, commandManager)
    {}

    // IntegrationTestBase interface
public:
    virtual QString testName();
    virtual void setup();
    virtual int doTest();
};

#endif // METADATACACHESAVETEST_H
