#ifndef DBIMAGECACHETESTS_H
#define DBIMAGECACHETESTS_H

#include <QObject>
#include <QtTest/QtTest>

class DbImageCacheTests: public QObject
{
    Q_OBJECT
private slots:
    void addAndRetrieveSmokeTest();
    void addAndRetrieveAfterSyncTest();
};

#endif // DBIMAGECACHETESTS_H
