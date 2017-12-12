#ifndef IMPORTLOSTMETADATATEST_H
#define IMPORTLOSTMETADATATEST_H

#include "integrationtestbase.h"

class ImportLostMetadataTest : public IntegrationTestBase
{
public:
    ImportLostMetadataTest(Commands::CommandManager *commandManager):
        IntegrationTestBase(commandManager)
    {}

    // IntegrationTestBase interface
public:
    virtual QString testName();
    virtual void setup();
    virtual int doTest();
};

#endif // IMPORTLOSTMETADATATEST_H
