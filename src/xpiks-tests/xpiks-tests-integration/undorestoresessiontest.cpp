#include "undorestoresessiontest.h"
#include "signalwaiter.h"
#include "testshelpers.h"
#include <QUrl>
#include <QList>
#include "xpikstestsapp.h"

QString UndoRestoreSessionTest::testName() {
    return QLatin1String("UndoRestoreSessionTest");
}

void UndoRestoreSessionTest::setup() {
    m_TestsApp.getSettingsModel().setUseSpellCheck(false);
    m_TestsApp.getSettingsModel().setSaveSession(true);
    m_TestsApp.getSettingsModel().setAutoFindVectors(true);
}

int UndoRestoreSessionTest::doTest() {
    QList<QUrl> sources;
    sources << setupFilePathForTest("images-for-tests/pixmap/img_0007.jpg")
            << setupFilePathForTest("images-for-tests/pixmap/seagull-for-clear.jpg")
            << setupFilePathForTest("images-for-tests/pixmap/seagull.jpg")
            << getDirPathForTest("images-for-tests/mixed/")
            << getDirPathForTest("images-for-tests/vector/");

    VERIFY(m_TestsApp.dropItemsForTest(sources), "Failed to add items");
    int addedCount = m_TestsApp.getArtworksCount();

    Models::SessionManager &sessionManager = m_TestsApp.getSessionManager();

    sleepWaitUntil(10, [&]() {
        return sessionManager.itemsCount() == addedCount;
    });
    VERIFY(sessionManager.itemsCount() == addedCount, "Session does not contain all files");

    m_TestsApp.deleteAllArtworks();
    LOG_DEBUG << "About to restore...";

    SignalWaiter waiter;
    m_TestsApp.connectWaiterForImport(waiter);
    int restoredCount = m_TestsApp.restoreSavedSession();

    if (this->getIsInMemoryOnly()) {
        VERIFY(restoredCount == 0, "Session restore should not work for memory-only mode");
        return 0;
    }

    VERIFY(sources.size() == restoredCount, "Failed to properly restore");
    VERIFY(m_TestsApp.continueReading(waiter), "Failed to reimport session");

    VERIFY(m_TestsApp.undoLastAction(), "Failed to Undo last action");

    VERIFY(m_TestsApp.getArtworksCount() == 0, "Items were not removed");

    return 0;
}
