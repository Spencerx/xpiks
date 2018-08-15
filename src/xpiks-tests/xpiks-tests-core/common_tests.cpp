#include "common_tests.h"
#include <Common/lrucache.h>
#include <Helpers/hashhelpers.h>

void CommonTests::lruCacheAddMoreTest() {
    size_t size = 3;
    Common::LRUCache<QString, int> cache(size);

    int times = 15;
    while (times--) {
        cache.put(QString("test %1").arg(times), times);
    }

    QCOMPARE(cache.size(), size);

    QCOMPARE(cache.get("test 0", -1), 0);
    QCOMPARE(cache.get("test 1", -1), 1);
    QCOMPARE(cache.get("test 2", -1), 2);

    QCOMPARE(cache.get("test 3", -1), -1);
    QCOMPARE(cache.get("test 4", -1), -1);
}

void CommonTests::lruCacheGetMissingTest() {
    Common::LRUCache<QString, int> cache(3);

    QCOMPARE(cache.get("test", -1), -1);
}

void CommonTests::lruCacheFrequentGetTest() {
    Common::LRUCache<QString, int> cache(3);

    int times = 15;
    while (times--) {
        cache.put(QString("test %1").arg(times), times);
        cache.get("test 14", -1);
    }

    QCOMPARE(cache.get("test 14", -1), 14);

    QCOMPARE(cache.get("test 0", -1), 0);
    QCOMPARE(cache.get("test 1", -1), 1);

    QCOMPARE(cache.get("test 2", -1), -1);
    QCOMPARE(cache.get("test 3", -1), -1);
    QCOMPARE(cache.get("test 4", -1), -1);
}

void CommonTests::lruCacheOnlyLastOneTest() {
    size_t size = 1;
    Common::LRUCache<QString, int> cache(size);

    int times = 15;
    while (times--) {
        cache.put(QString("test %1").arg(times), times);
    }

    cache.put("the only one", 123);

    QCOMPARE(cache.size(), size);

    QCOMPARE(cache.get("the only one", -1), 123);

    times = 15;
    while (times--) {
        QCOMPARE(cache.get(QString("test %1").arg(times), -1), -1);
    }
}

void CommonTests::lruCacheAddEnoughTest() {
    int times = 15;
    Common::LRUCache<QString, int> cache(times);

    while (times--) {
        cache.put(QString("test %1").arg(times), times);
    }

    times = 15;
    QCOMPARE(cache.size(), (size_t)times);

    while (times--) {
        QVERIFY(cache.get(QString("test %1").arg(times), -1) != -1);
    }
}
