#ifndef DELETEOLDLOGSTEST_H
#define DELETEOLDLOGSTEST_H

#include <QObject>
#include <QtTest>  // IWYU pragma: keep
// IWYU pragma: no_include <QString>

class DeleteOldLogsTest: public QObject
{
    Q_OBJECT
private slots:
    void deleteNoLogsTest();
    void dontDeleteTest();
    void deleteOldTest();
    void deleteLargeTest();
    void deleteManyTest();
    void deleteCombinedTest();
};

#endif // DELETEOLDLOGSTEST_H
