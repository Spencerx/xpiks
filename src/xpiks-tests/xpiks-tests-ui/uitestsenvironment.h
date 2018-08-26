#ifndef UITESTSENVIRONMENT_H
#define UITESTSENVIRONMENT_H

#include <Common/systemenvironment.h>

class UITestsEnvironment: public Common::SystemEnvironment
{
public:
    UITestsEnvironment(const QStringList &appArguments = QStringList());
};

#endif // UITESTSENVIRONMENT_H
