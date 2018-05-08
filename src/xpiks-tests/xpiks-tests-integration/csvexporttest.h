#ifndef CSVEXPORTTEST_H
#define CSVEXPORTTEST_H

#include "integrationtestbase.h"

class CsvExportTest : public IntegrationTestBase
{
public:
    CsvExportTest(IntegrationTestsEnvironment &environment, Commands::CommandManager *commandManager):
        IntegrationTestBase(environment, commandManager)
    {}

    // IntegrationTestBase interface
public:
    virtual QString testName();
    virtual void setup();
    virtual int doTest();
};

#endif // CSVEXPORTTEST_H
