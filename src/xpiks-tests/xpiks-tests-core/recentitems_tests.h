#ifndef RECENTITEMSTESTS_H
#define RECENTITEMSTESTS_H

#include <QObject>
#include <QtTest>  // IWYU pragma: keep
// IWYU pragma: no_include <QString>

class RecentItemsTests : public QObject
{
    Q_OBJECT
private slots:
    void pushMoreThanXFilesTest();
    void pushMoreThanXDirectoriesTest();
    void lastPushedIsMostRecentFileTest();
    void lastPushedIsMostRecentDirectoryTest();
};

#endif // RECENTITEMSTESTS_H
