#ifndef USERDICTEDITTEST_H
#define USERDICTEDITTEST_H

#include "integrationtestbase.h"

namespace SpellCheck {
    class UserDictEditModel;
}

class UserDictEditTest : public IntegrationTestBase
{
public:
    UserDictEditTest(IntegrationTestsEnvironment &environment, XpiksTestsApp &testsApp):
        IntegrationTestBase(environment, testsApp)
    {}

    // IntegrationTestBase interface
public:
    virtual QString testName();
    virtual void setup();
    virtual int doTest();
};

#endif // USERDICTEDITTEST_H
