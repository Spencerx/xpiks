#ifndef USERDICTEDITTEST_H
#define USERDICTEDITTEST_H

#include "integrationtestbase.h"

namespace SpellCheck {
    class UserDictEditModel;
}

class UserDictEditTest : public IntegrationTestBase
{
public:
    UserDictEditTest(IntegrationTestsEnvironment &environment, Commands::CommandManager *commandManager):
        IntegrationTestBase(environment, commandManager)
    {}

    // IntegrationTestBase interface
public:
    virtual QString testName();
    virtual void setup();
    virtual int doTest();

private:
    SpellCheck::UserDictEditModel *m_UserDictEditModel;
};

#endif // USERDICTEDITTEST_H
