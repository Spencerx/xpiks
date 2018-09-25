#include "savewithemptytitletest.h"
#include <QUrl>
#include <QList>
#include "signalwaiter.h"
#include "xpikstestsapp.h"

QString SaveWithEmptyTitleTest::testName() {
    return QLatin1String("SaveWithEmptyTitleTest");
}

void SaveWithEmptyTitleTest::setup() {
}

int SaveWithEmptyTitleTest::doTest() {
    QList<QUrl> files;
    files << setupFilePathForTest("images-for-tests/pixmap/seagull.jpg");

    VERIFY(m_TestsApp.addFilesForTest(files), "Failed to add files");

    auto artwork = m_TestsApp.getArtwork(0);
    auto image = std::dynamic_pointer_cast<Artworks::ImageArtwork>(artwork);

    VERIFY(image->getImageSize().width() == 1920, "Image width was read incorrectly");
    VERIFY(image->getImageSize().height() == 1272, "Image height was read incorrectly");

    QString description = "Description for title";
    artwork->setDescription(description);
    artwork->setTitle(""); // title should be taken from description

    artwork->setIsSelected(true);
    SignalWaiter waiter;
    m_TestsApp.connectWaiterForExport(waiter);
    m_TestsApp.dispatch(QMLExtensions::UICommandID::SetupExportMetadata);

    VERIFY(waiter.wait(20), "Timeout exceeded for writing metadata.");
    VERIFY(m_TestsApp.checkExportSucceeded(), "Failed to export artworks");

    // --

    m_TestsApp.deleteAllArtworks();
    VERIFY(m_TestsApp.getArtworksCount() == 0, "Failed to remove files");
    VERIFY(m_TestsApp.addFilesForTest(files), "Failed to add files");

    artwork = m_TestsApp.getArtwork(0);
    VERIFY(description == artwork->getTitle(), "Title was not set from description");

    return 0;
}
