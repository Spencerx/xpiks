#ifndef UITESTSENVIRONMENT_H
#define UITESTSENVIRONMENT_H

#include <Common/isystemenvironment.h>
#include <QDir>

class UITestsEnvironment: public Common::ISystemEnvironment
{
public:
    UITestsEnvironment() {}

    // ISystemEnvironment interface
public:
    virtual QString root() const override { return QString(); }
    virtual QString path(const QStringList &path) override { return path.join(QDir::separator()); }
    virtual bool ensureDirExists(const QString &) override { return true; }
    virtual bool getIsInMemoryOnly() const override { return true; }
    virtual bool getIsRecoveryMode() const override { return false; }
};

#endif // UITESTSENVIRONMENT_H
