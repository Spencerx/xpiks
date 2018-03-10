#ifndef TESTSHELPERS_H
#define TESTSHELPERS_H

#include <functional>
#include <QString>

void sleepWaitUntil(int seconds, const std::function<bool ()> &condition);
QString findFullPathForTests(const QString &prefix);
QString findWildcartPathForTests(const QString &dirPath, const QStringList &filters);

#endif // TESTSHELPERS_H
