#include "removedirsavessessiontest.h"

#include "signalwaiter.h"
#include "testshelpers.h"
#include "xpikstestsapp.h"

QString RemoveDirSavesSessionTest::testName() {
    return QLatin1String("RemoveDirSavesSessionTest");
}

void RemoveDirSavesSessionTest::setup() {
    m_TestsApp.getSettingsModel().setSaveSession(true);
}

int RemoveDirSavesSessionTest::doTest() {
    QList<QUrl> sources;
    sources << getDirPathForTest("images-for-tests/mixed/")
            << getDirPathForTest("images-for-tests/vector/");

    VERIFY(m_TestsApp.dropItemsForTest(sources), "Failed to add items");
    int addedCount = m_TestsApp.getArtworksCount();

    Models::SessionManager &sessionManager = m_TestsApp.getSessionManager();

    sleepWaitUntil(5, [&]() { return sessionManager.itemsCount() == addedCount; });
    VERIFY(sessionManager.itemsCount() == addedCount, "Session does not contain all files");

    const int secondDirCount = m_TestsApp.getArtworksRepository().getFilesCountForDirectory(1);
    m_TestsApp.removeDirectory(0);

    sleepWaitUntil(5, [&]() { return sessionManager.itemsCount() == secondDirCount; });
    VERIFY(sessionManager.itemsCount() == secondDirCount, "Removed directory was accounted again");

    return 0;
}
