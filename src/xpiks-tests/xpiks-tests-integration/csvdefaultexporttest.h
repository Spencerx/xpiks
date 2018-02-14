#ifndef CSVDEFAULTEXPORTTEST_H
#define CSVDEFAULTEXPORTTEST_H

#include "integrationtestbase.h"

class CsvDefaultExportTest : public IntegrationTestBase
{
public:
    CsvDefaultExportTest(Commands::CommandManager *commandManager):
        IntegrationTestBase(commandManager)
    {}

    // IntegrationTestBase interface
public:
    virtual QString testName();
    virtual void setup();
    virtual int doTest();
};

#endif // CSVDEFAULTEXPORTTEST_H
