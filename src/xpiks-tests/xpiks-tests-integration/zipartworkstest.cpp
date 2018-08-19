#include "zipartworkstest.h"
#include <QUrl>
#include <QList>
#include "signalwaiter.h"
#include <Helpers/filehelpers.h>
#include "xpikstestsapp.h"

QString ZipArtworksTest::testName() {
    return QLatin1String("ZipArtworksTest");
}

void ZipArtworksTest::setup() {
    m_TestsApp.getSettingsModel().setAutoFindVectors(true);
}

int ZipArtworksTest::doTest() {
    QList<QUrl> files;
    files << setupFilePathForTest("images-for-tests/vector/026.jpg");
    files << setupFilePathForTest("images-for-tests/vector/027.jpg");
    files << setupFilePathForTest("images-for-tests/pixmap/seagull.jpg");

    VERIFY(m_TestsApp.addFilesForTest(files), "Failed to add files");

    m_TestsApp.selectAllArtworks();
    m_TestsApp.dispatch(QMLExtensions::UICommandID::ZipSelected);

    Models::ZipArchiver &zipArchiver = m_TestsApp.getZipArchiver();
    SignalWaiter waiter;
    QObject::connect(&zipArchiver, &Models::ZipArchiver::finishedProcessing,
                     &waiter, &SignalWaiter::finished);

    zipArchiver.archiveArtworks();

    VERIFY(waiter.wait(20), "Timeout while zipping artworks");

    VERIFY(zipArchiver.getItemsCount() == 2, "ZipArchiver didn't get all the files");
    VERIFY(!zipArchiver.getHasErrors(), "Errors while zipping");

    for (int i = 0; i < files.length(); ++i) {
        auto artwork = m_TestsApp.getArtwork(i);
        auto image = std::dynamic_pointer_cast<Artworks::ImageArtwork>(artwork);
        Q_ASSERT(image != nullptr);

        if (image->hasVectorAttached()) {
            QString zipPath = Helpers::getArchivePath(artwork->getFilepath());
            VERIFY(QFileInfo(zipPath).exists(), "Zip file not found");
        }
    }

    return 0;
}


