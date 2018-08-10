#ifndef SPELLCHECKMULTIREPLACETEST_H
#define SPELLCHECKMULTIREPLACETEST_H

#include "integrationtestbase.h"

class SpellCheckMultireplaceTest : public IntegrationTestBase
{
public:
    SpellCheckMultireplaceTest(IntegrationTestsEnvironment &environment, XpiksTestsApp &testsApp):
        IntegrationTestBase(environment, testsApp)
    {}

    // IntegrationTestBase interface
public:
    virtual QString testName();
    virtual void setup();
    virtual int doTest();
};

#endif // SPELLCHECKMULTIREPLACETEST_H
