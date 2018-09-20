#ifndef STOCKFTPAUTOCOMPLETETEST_H
#define STOCKFTPAUTOCOMPLETETEST_H

#include "integrationtestbase.h"

class StockFtpAutoCompleteTest : public IntegrationTestBase
{
public:
    StockFtpAutoCompleteTest(IntegrationTestsEnvironment &environment, XpiksTestsApp &testsApp):
        IntegrationTestBase(environment, testsApp)
    {}

    // IntegrationTestBase interface
public:
    virtual QString testName();
    virtual void setup();
    virtual int doTest();
};

#endif // STOCKFTPAUTOCOMPLETETEST_H
