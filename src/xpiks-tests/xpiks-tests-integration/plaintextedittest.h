#ifndef PLAINTEXTEDITTEST_H
#define PLAINTEXTEDITTEST_H

#include "integrationtestbase.h"

class PlainTextEditTest : public IntegrationTestBase
{
public:
    PlainTextEditTest(IntegrationTestsEnvironment &environment, XpiksTestsApp &testsApp):
        IntegrationTestBase(environment, testsApp)
    {}

    // IntegrationTestBase interface
public:
    virtual QString testName();
    virtual void setup();
    virtual int doTest();
};

#endif // PLAINTEXTEDITTEST_H
