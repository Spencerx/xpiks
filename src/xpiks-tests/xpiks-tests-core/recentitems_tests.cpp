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

class RecentItemsTestModel: public Models::RecentItemsModel
{
public:
    using Models::RecentItemsModel::RecentItemsModel;
    virtual void initialize() override {}
    virtual void sync() override {}
};

void RecentItemsTests::leastUsedIsRemovedTest() {
    RecentItemsTestModel recentItems(3);

    recentItems.pushItem("d1");
    recentItems.pushItem("d2");
    recentItems.pushItem("d3");

    // also "recently use" d1
    recentItems.pushItem("d1");
    recentItems.pushItem("d4");

    QVERIFY(recentItems.contains("d1"));
}

void RecentItemsTests::serializeDeserializeTest() {
    RecentItemsTestModel recentFrom(3);

    recentFrom.pushItem("d1");
    recentFrom.pushItem("d2");
    recentFrom.pushItem("d3");

    QString serialized = recentFrom.serializeItems();

    RecentItemsTestModel recentTo(3);

    recentTo.deserializeItems(serialized);

    QCOMPARE(recentFrom.rowCount(), recentTo.rowCount());
    QCOMPARE(recentFrom.getLatestUsedItem(), recentTo.getLatestUsedItem());
}
