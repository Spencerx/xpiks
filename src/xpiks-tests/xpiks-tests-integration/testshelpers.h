#ifndef TESTSHELPERS_H
#define TESTSHELPERS_H

#include <functional>
#include <QString>
#include <QUrl>

void sleepWaitUntil(int seconds, const std::function<bool ()> &condition);
QString findFullPathForTests(const QString &prefix);
bool tryFindFullPathForTests(const QString &prefix, QString &path);
QString findWildcartPathForTests(const QString &dirPath, const QStringList &filters);
QUrl setupFilePathForTest(const QString &root, const QString &prefix, bool withVector=false);

#endif // TESTSHELPERS_H
