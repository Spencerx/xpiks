#ifndef INTEGRATIONTESTSENVIRONMENT_H
#define INTEGRATIONTESTSENVIRONMENT_H

#include "../../xpiks-qt/Common/systemenvironment.h"

class IntegrationTestsEnvironment: public Common::SystemEnvironment
{
public:
    IntegrationTestsEnvironment(const QStringList &appArguments = QStringList());

public:
    virtual void ensureSystemDirectoriesExist() override;
    QString getSessionRoot() { return m_SessionRoot; }

private:
    QString m_SessionRoot;
};

#endif // INTEGRATIONTESTSENVIRONMENT_H
