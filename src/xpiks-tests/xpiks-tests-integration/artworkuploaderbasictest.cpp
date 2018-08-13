#include "artworkuploaderbasictest.h"
#include <QUrl>
#include <QList>
#include "integrationtestbase.h"
#include "signalwaiter.h"
#include "xpikstestsapp.h"
#include <Models/Connectivity/artworksuploader.h>

QString ArtworkUploaderBasicTest::testName() {
    return QLatin1String("ArtworkUploaderBasicTest");
}

void ArtworkUploaderBasicTest::setup() {
}

int ArtworkUploaderBasicTest::doTest() {
    QList<QUrl> files;
    files << setupFilePathForTest("images-for-tests/mixed/026.jpg");
    files << setupFilePathForTest("images-for-tests/mixed/027.jpg");
    files << setupFilePathForTest("images-for-tests/mixed/0267.jpg");

    VERIFY(m_TestsApp.addFilesForTest(files), "Failed to add files");

    m_TestsApp.dispatch(QMLExtensions::UICommandID::UploadSelected);

    VERIFY(m_TestsApp.getArtworksUploader().getItemsCount() == files.size(), "Artworks didn't get to the uploader");

    // TODO: add actual uploading here

    return 0;
}
