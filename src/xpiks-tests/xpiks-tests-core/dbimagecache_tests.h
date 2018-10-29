#ifndef DBIMAGECACHETESTS_H
#define DBIMAGECACHETESTS_H

#include <QObject>
#include <QtTest>  // IWYU pragma: keep
// IWYU pragma: no_include <QString>

class DbImageCacheTests: public QObject
{
    Q_OBJECT
private slots:
    void addAndRetrieveSmokeTest();
    void addAndRetrieveAfterSyncTest();
};

#endif // DBIMAGECACHETESTS_H
