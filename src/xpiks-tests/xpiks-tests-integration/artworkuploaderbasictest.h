#ifndef ARTWORKUPLOADERBASICTEST_H
#define ARTWORKUPLOADERBASICTEST_H

#include "integrationtestbase.h"

class ArtworkUploaderBasicTest:
        public IntegrationTestBase
{
public:
    ArtworkUploaderBasicTest(IntegrationTestsEnvironment &environment, Commands::CommandManager *commandManager):
        IntegrationTestBase(environment, commandManager)
    {}

    // IntegrationTestBase interface

public:
    virtual QString testName();
    virtual void setup();
    virtual int doTest();
};

#endif // ARTWORKUPLOADERBASICTEST_H
