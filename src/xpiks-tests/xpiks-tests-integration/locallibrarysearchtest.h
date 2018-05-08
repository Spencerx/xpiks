#ifndef LOCALLIBRARYSEARCHTEST_H
#define LOCALLIBRARYSEARCHTEST_H

#include "integrationtestbase.h"

class LocalLibrarySearchTest : public IntegrationTestBase
{
public:
    LocalLibrarySearchTest(IntegrationTestsEnvironment &environment, Commands::CommandManager *commandManager):
        IntegrationTestBase(environment, commandManager)
    {}

    // IntegrationTestBase interface
public:
    virtual QString testName();
    virtual void setup();
    virtual int doTest();
};

#endif // LOCALLIBRARYSEARCHTEST_H
