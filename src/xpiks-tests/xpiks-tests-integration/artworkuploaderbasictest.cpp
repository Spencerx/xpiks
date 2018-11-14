#include "artworkuploaderbasictest.h"

#include <QLatin1String>
#include <QList>
#include <QUrl>

#include "Models/Connectivity/artworksuploader.h"
#include "QMLExtensions/uicommandid.h"

#include "xpikstestsapp.h"

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

    m_TestsApp.selectAllArtworks();
    m_TestsApp.dispatch(QMLExtensions::UICommandID::SetupUpload);

    VERIFY(m_TestsApp.getArtworksUploader().getItemsCount() == files.size(), "Artworks didn't get to the uploader");

    // TODO: add actual uploading here

    return 0;
}
