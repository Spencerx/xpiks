#ifndef XPIKSTESTSAPP_H
#define XPIKSTESTSAPP_H

#include "../../xpiks-qt/xpiksapp.h"

class XpiksTestsApp: public XpiksApp
{
public:
    XpiksTestsApp(Common::ISystemEnvironment &environment);

public:
    Commands::CommandManager &getCommandManager() { return m_CommandManager; }

public:
    void waitInitialized();

public:
    virtual void initialize() override;
};

#endif // XPIKSTESTSAPP_H
