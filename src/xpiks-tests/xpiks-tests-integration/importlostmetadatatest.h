#ifndef IMPORTLOSTMETADATATEST_H
#define IMPORTLOSTMETADATATEST_H

#include "integrationtestbase.h"

class ImportLostMetadataTest : public IntegrationTestBase
{
public:
    ImportLostMetadataTest(IntegrationTestsEnvironment &environment, XpiksTestsApp &testsApp):
        IntegrationTestBase(environment, testsApp)
    {}

    // IntegrationTestBase interface
public:
    virtual QString testName();
    virtual void setup();
    virtual int doTest();
};

#endif // IMPORTLOSTMETADATATEST_H
