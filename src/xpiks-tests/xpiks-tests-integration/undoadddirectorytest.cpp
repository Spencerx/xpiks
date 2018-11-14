#include "undoadddirectorytest.h"

#include <QLatin1String>
#include <QList>
#include <QUrl>

#include "Helpers/indicesranges.h"
#include "Models/settingsmodel.h"

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

    SignalWaiter waiter;
    m_TestsApp.connectWaiterForImport(waiter);
    const int addedCount = m_TestsApp.addDirectories(dirs);
    VERIFY(waiter.wait(20), "Timeout exceeded for reading metadata on first import");
    VERIFY(m_TestsApp.checkImportSucceeded(), "Failed to auto import");

    // remove 2 artworks
    m_TestsApp.deleteArtworks(Helpers::IndicesRanges({0, 1}));
    VERIFY(m_TestsApp.getArtworksCount() == addedCount - 2, "Artworks were not removed");

    // remove directory
    m_TestsApp.removeDirectory(0);
    VERIFY(m_TestsApp.getArtworksCount() == 0, "All items were not removed");

    VERIFY(m_TestsApp.undoLastAction(), "Failed to undo remove directory");
    VERIFY(waiter.wait(20), "Timeout exceeded for auto import");
    VERIFY(m_TestsApp.checkImportSucceeded(), "Failed to auto import");
    VERIFY(m_TestsApp.getArtworksCount() == addedCount, "Items were not put back");

    return 0;
}
