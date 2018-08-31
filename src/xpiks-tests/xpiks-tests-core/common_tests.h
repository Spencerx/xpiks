#ifndef COMMON_TESTS_H
#define COMMON_TESTS_H

#include <QtTest/QtTest>

class CommonTests: public QObject
{
    Q_OBJECT
private slots:
    void lruCacheAddMoreTest();
    void lruCacheGetMissingTest();
    void lruCacheFrequentGetTest();
    void lruCacheOnlyLastOneTest();
    void lruCacheAddEnoughTest();
    void lruCacheAddManyMoreTest();
};

#endif // COMMON_TESTS_H
