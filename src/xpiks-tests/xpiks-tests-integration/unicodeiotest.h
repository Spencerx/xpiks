#ifndef UNICODEIOTEST_H
#define UNICODEIOTEST_H

#include "integrationtestbase.h"

class UnicodeIoTest : public IntegrationTestBase
{
public:
    UnicodeIoTest(IntegrationTestsEnvironment &environment, XpiksTestsApp &testsApp):
        IntegrationTestBase(environment, testsApp)
    {}

    // IntegrationTestBase interface
public:
    virtual QString testName();
    virtual void setup();
    virtual int doTest();
};

#endif // UNICODEIOTEST_H
