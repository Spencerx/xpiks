#include "autoattachvectorstest.h"

#include <memory>

#include <QLatin1String>
#include <QList>
#include <QUrl>

#include "Artworks/imageartwork.h"
#include "Models/settingsmodel.h"

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
        auto artwork = m_TestsApp.getArtwork(i);
        auto image = std::dynamic_pointer_cast<Artworks::ImageArtwork>(artwork);
        VERIFY(image != nullptr && image->hasVectorAttached(), "Vector is not attached!");
    }

    return 0;
}
