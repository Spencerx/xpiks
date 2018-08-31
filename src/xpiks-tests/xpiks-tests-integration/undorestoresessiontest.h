#ifndef UNDORESTORESESSIONTEST_H
#define UNDORESTORESESSIONTEST_H

#include "integrationtestbase.h"

class UndoRestoreSessionTest: public IntegrationTestBase
{
public:
    UndoRestoreSessionTest(IntegrationTestsEnvironment &environment, XpiksTestsApp &testsApp):
        IntegrationTestBase(environment, testsApp)
    {}

    // IntegrationTestBase interface
public:
    virtual QString testName();
    virtual void setup();
    virtual int doTest();
};

#endif // UNDORESTORESESSIONTEST_H
