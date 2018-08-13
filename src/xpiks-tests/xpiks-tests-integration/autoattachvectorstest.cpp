#include "autoattachvectorstest.h"
#include <QUrl>
#include <QList>
#include "integrationtestbase.h"
#include "signalwaiter.h"
#include "xpikstestsapp.h"

QString AutoAttachVectorsTest::testName() {
    return QLatin1String("AutoAttachVectorsTest");
}

void AutoAttachVectorsTest::setup() {
    m_TestsApp.getSettingsModel().setAutoFindVectors(true);

    // copy files
    setupFilePathForTest("images-for-tests/vector/026.eps");
    setupFilePathForTest("images-for-tests/vector/027.eps");
}

int AutoAttachVectorsTest::doTest() {
    QList<QUrl> files;
    files << setupFilePathForTest("images-for-tests/vector/026.jpg");
    files << setupFilePathForTest("images-for-tests/vector/027.jpg");

    VERIFY(m_TestsApp.addFilesForTest(files), "Failed to add files");

    for (int i = 0; i < files.length(); ++i) {
        Artworks::ArtworkMetadata *artwork = m_TestsApp.getArtwork(i);
        Artworks::ImageArtwork *image = dynamic_cast<Artworks::ImageArtwork *>(artwork);
        VERIFY(image != NULL && image->hasVectorAttached(), "Vector is not attached!");
    }

    return 0;
}
