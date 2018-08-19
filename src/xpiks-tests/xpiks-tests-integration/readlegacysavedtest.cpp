#include "readlegacysavedtest.h"
#include <QUrl>
#include <QList>
#include "integrationtestbase.h"
#include "signalwaiter.h"
#include "xpikstestsapp.h"

QString ReadLegacySavedTest::testName() {
    return QLatin1String("ReadLegacySavedTest");
}

void ReadLegacySavedTest::setup() {
    m_TestsApp.getSettingsModel().setAutoFindVectors(false);
}

int ReadLegacySavedTest::doTest() {
    QList<QUrl> files;
    files << setupFilePathForTest("images-for-tests/pixmap/img_0007.jpg");

    VERIFY(m_TestsApp.addFilesForTest(files), "Failed to add files");

    auto artwork = m_TestsApp.getArtwork(0);
    const QStringList &keywords = artwork->getKeywords();

    QStringList expectedKeywords = QString("rock,nature,landscape,white,background,beautiful,sun,light,mountain,outdoor,top,rocky,snow,fog,horizon").split(',');

    auto image = std::dynamic_pointer_cast<Artworks::ImageArtwork>(artwork);

    VERIFY(expectedKeywords == keywords, "Keywords are not the same!");
    VERIFY(image->getImageSize().width() == 2752, "Image width was read incorrectly");
    VERIFY(image->getImageSize().height() == 2794, "Image height was read incorrectly");

    return 0;
}
