#include "recentitems_tests.h"

#include "Models/Session/recentdirectoriesmodel.h"
#include "Models/Session/recentfilesmodel.h"
#include "Models/Session/recentitemsmodel.h"

#include "Mocks/coretestsenvironment.h"

void checkCannotPushMoreThan(Models::RecentItemsModel &recentItems) {
    int maxRecent = recentItems.getMaxRecentItems();

    for (int i = 0; i < maxRecent*2; ++i) {
        recentItems.pushItem("/directory/path/" + QString::number(i));
    }

    QCOMPARE(recentItems.rowCount(), maxRecent);
}

void RecentItemsTests::pushMoreThanXFilesTest() {
    Mocks::CoreTestsEnvironment environment;
    Models::RecentFilesModel recentFiles(environment);
    recentFiles.initialize();

    checkCannotPushMoreThan(recentFiles);
}

void RecentItemsTests::pushMoreThanXDirectoriesTest() {
    Mocks::CoreTestsEnvironment environment;
    Models::RecentDirectoriesModel recentDirectories(environment);
    recentDirectories.initialize();

    checkCannotPushMoreThan(recentDirectories);
}

void checkLastPushedIsMostRecent(Models::RecentItemsModel &recentItems) {
    QString item = "/path/to/test/dir";
    recentItems.pushItem(item);

    QCOMPARE(recentItems.getLatestUsedItem(), item);

    QString item2 = "/path/to/other/dir/here";
    recentItems.pushItem(item2);

    QCOMPARE(recentItems.getLatestUsedItem(), item2);
}

void RecentItemsTests::lastPushedIsMostRecentFileTest() {
    Mocks::CoreTestsEnvironment environment;
    Models::RecentFilesModel recentFiles(environment);
    recentFiles.initialize();

    checkLastPushedIsMostRecent(recentFiles);
}

void RecentItemsTests::lastPushedIsMostRecentDirectoryTest() {
    Mocks::CoreTestsEnvironment environment;
    Models::RecentDirectoriesModel recentDirectories(environment);
    recentDirectories.initialize();

    checkLastPushedIsMostRecent(recentDirectories);
}
