#include "undorestoresessiontest.h"
#include "../../xpiks-qt/Models/settingsmodel.h"
#include "../../xpiks-qt/Models/sessionmanager.h"
#include "../../xpiks-qt/MetadataIO/metadataiocoordinator.h"
#include "../../xpiks-qt/MetadataIO/artworkssnapshot.h"
#include "../../xpiks-qt/Models/artitemsmodel.h"
#include "../../xpiks-qt/Models/artworksrepository.h"
#include "../../xpiks-qt/Models/imageartwork.h"
#include "../../xpiks-qt/UndoRedo/undoredomanager.h"
#include "signalwaiter.h"
#include "testshelpers.h"

QString UndoRestoreSessionTest::testName() {
    return QLatin1String("UndoRestoreSessionTest");
}

void UndoRestoreSessionTest::setup() {
    Models::SettingsModel *settingsModel = m_TestsApp.getSettingsModel();

    m_TestsApp.getSettingsModel().setUseSpellCheck(false);
    m_TestsApp.getSettingsModel().setSaveSession(true);
    m_TestsApp.getSettingsModel().setAutoFindVectors(true);
}

int UndoRestoreSessionTest::doTest() {
    Models::ArtItemsModel *artItemsModel = m_TestsApp.getArtItemsModel();
    Models::SessionManager *sessionManager = m_TestsApp.getSessionManager();
    //VERIFY(sessionManager->itemsCount() == 0, "Session is not cleared");
    Models::ArtworksRepository *artworksRepository = m_TestsApp.getArtworksRepository();

    QList<QUrl> sources;
    sources << setupFilePathForTest("images-for-tests/pixmap/img_0007.jpg")
            << setupFilePathForTest("images-for-tests/pixmap/seagull-for-clear.jpg")
            << setupFilePathForTest("images-for-tests/pixmap/seagull.jpg")
            << getDirPathForTest("images-for-tests/mixed/")
            << getDirPathForTest("images-for-tests/vector/");

    MetadataIO::MetadataIOCoordinator *ioCoordinator = m_TestsApp.getMetadataIOCoordinator();
    SignalWaiter waiter;
    QObject::connect(ioCoordinator, SIGNAL(metadataReadingFinished()), &waiter, SIGNAL(finished()));

    int addedCount = artItemsModel->dropFiles(sources);
    ioCoordinator->continueReading(true);

    VERIFY(waiter.wait(20), "Timeout exceeded for reading metadata.");

    VERIFY(!ioCoordinator->getHasErrors(), "Errors in IO Coordinator while reading");

    sleepWaitUntil(10, [&]() {
        return sessionManager->itemsCount() == addedCount;
    });
    VERIFY(sessionManager->itemsCount() == addedCount, "Session does not contain all files");

    artworksRepository->resetEverything();
    artItemsModel->fakeDeleteAllItems();
    LOG_DEBUG << "About to restore...";

    int restoredCount = sessionManager->restoreSession(*artworksRepository);
    if (this->getIsInMemoryOnly()) {
        VERIFY(restoredCount == 0, "Session restore should not work in memory-only mode");
        return 0;
    }

    VERIFY(addedCount == restoredCount, "Failed to properly restore");
    ioCoordinator->continueReading(true);

    VERIFY(waiter.wait(20), "Timeout exceeded for reading session metadata.");

    VERIFY(!ioCoordinator->getHasErrors(), "Errors in IO Coordinator while reading");

    UndoRedo::UndoRedoManager *undoRedoManager = m_TestsApp.getUndoRedoManager();

    bool undoSuccess = undoRedoManager->undoLastAction();
    VERIFY(undoSuccess, "Failed to Undo last action");

    VERIFY(m_TestsApp.getArtworksCount() == 0, "Items were not removed");

    return 0;
}
