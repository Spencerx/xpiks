#include "dbimagecache_tests.h"
#include "../../xpiks-qt/QMLExtensions/dbimagecacheindex.h"
#include "Mocks/coretestsenvironment.h"
#include "../../xpiks-qt/Storage/databasemanager.h"
#include "../../xpiks-qt/QMLExtensions/cachedimage.h"

#define DECLARE_AND_INIT\
    Mocks::CoreTestsEnvironment environment;\
    Storage::DatabaseManager dbManager(environment);\
    dbManager.initialize();\
    QMLExtensions::DbImageCacheIndex imageCache(&dbManager);\
    imageCache.initialize();

#define FINALIZE\
    imageCache.finalize();

void DbImageCacheTests::addAndRetrieveSmokeTest() {
    DECLARE_AND_INIT;

    const QString path = "/path/to/an/image.jpg";
    const QString thumbPath = "/path/to/random/thumbnail.jpg";
    QMLExtensions::CachedImage cachedImage;
    cachedImage.m_Filename = thumbPath;
    cachedImage.m_LastModified = QDateTime::currentDateTimeUtc();
    cachedImage.m_Size = QSize(1024, 700);

    imageCache.update(path, cachedImage);

    QMLExtensions::CachedImage foundImage;
    bool found = imageCache.tryGet(path, foundImage);
    QVERIFY2(found, "Image has not been found in cache");
    QCOMPARE(foundImage.m_Filename, thumbPath);

    FINALIZE;
}

void DbImageCacheTests::addAndRetrieveAfterSyncTest() {
    DECLARE_AND_INIT;

    const QString path = "/path/to/an/image.jpg";
    const QString thumbPath = "/path/to/random/thumbnail.jpg";
    QMLExtensions::CachedImage cachedImage;
    cachedImage.m_Filename = thumbPath;
    cachedImage.m_LastModified = QDateTime::currentDateTimeUtc();
    cachedImage.m_Size = QSize(1024, 700);

    imageCache.update(path, cachedImage);
    imageCache.sync();

    QMLExtensions::CachedImage foundImage;
    bool found = imageCache.tryGet(path, foundImage);
    QVERIFY2(found, "Image has not been found in cache after sync");
    QCOMPARE(foundImage.m_Filename, thumbPath);

    FINALIZE;
}
