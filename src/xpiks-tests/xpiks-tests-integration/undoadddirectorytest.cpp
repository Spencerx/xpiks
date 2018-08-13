#include "undoadddirectorytest.h"
#include <QUrl>
#include "signalwaiter.h"
#include "xpikstestsapp.h"

QString UndoAddDirectoryTest::testName() {
    return QLatin1String("UndoAddDirectoryTest");
}

void UndoAddDirectoryTest::setup() {
    m_TestsApp.getSettingsModel().setUseAutoImport(true);
}

int UndoAddDirectoryTest::doTest() {
    QList<QUrl> dirs;
    dirs << getDirPathForTest("images-for-tests/mixed/");

    VERIFY(m_TestsApp.addDirectoriesForTest(dirs), "Failed to add directories");
    int artworksCount = m_TestsApp.getArtworksCount();

    // remove 2 artworks
    m_TestsApp.deleteArtworks(Helpers::IndicesRanges({0, 1}));
    VERIFY(m_TestsApp.getArtworksCount() == artworksCount - 2, "Artworks were not removed");

    // remove directory
    m_TestsApp.deleteArtworksFromDirectory(0);
    VERIFY(m_TestsApp.getArtworksCount() == 0, "All items were not removed");

    SignalWaiter waiter;
    m_TestsApp.connectWaiterForImport(waiter);
    VERIFY(m_TestsApp.undoLastAction(), "Failed to undo remove directory");
    VERIFY(m_TestsApp.continueReading(waiter), "Failed to reimport files");
    VERIFY(m_TestsApp.getArtworksCount() == artworksCount, "Items were not put back");

    return 0;
}
