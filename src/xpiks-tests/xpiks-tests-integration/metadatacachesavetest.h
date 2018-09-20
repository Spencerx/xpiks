#ifndef METADATACACHESAVETEST_H
#define METADATACACHESAVETEST_H

#include "integrationtestbase.h"

class MetadataCacheSaveTest : public IntegrationTestBase
{
public:
    MetadataCacheSaveTest(IntegrationTestsEnvironment &environment, XpiksTestsApp &testsApp):
        IntegrationTestBase(environment, testsApp)
    {}

    // IntegrationTestBase interface
public:
    virtual QString testName();
    virtual void setup();
    virtual int doTest();
};

#endif // METADATACACHESAVETEST_H
