#include "addfilesbasictest.h"
#include <QUrl>
#include <QFileInfo>
#include <QStringList>
#include "integrationtestbase.h"
#include "signalwaiter.h"
#include "xpikstestsapp.h"
#include <Models/Artworks/artworkslistmodel.h>
#include <Artworks/artworkmetadata.h>
#include <Artworks/imageartwork.h>

QString AddFilesBasicTest::testName() {
    return QLatin1String("AddFilesBasicTest");
}

void AddFilesBasicTest::setup() {
    m_TestsApp.getSettingsModel().setAutoFindVectors(false);
}

int AddFilesBasicTest::doTest() {
    QList<QUrl> files;
    files << setupFilePathForTest("images-for-tests/vector/026.jpg");

    VERIFY(m_TestsApp.addFilesForTest(files), "Failed to add files");

    auto *artwork = m_TestsApp.getArtwork(0);
    const QStringList &keywords = artwork->getKeywords();

    QStringList expectedKeywords = QString("abstract,art,black,blue,creative,dark,decor,decoration,decorative,design,dot,drops,elegance,element,geometric,interior,light,modern,old,ornate,paper,pattern,purple,retro,seamless,style,textile,texture,vector,wall,wallpaper").split(',');

    Artworks::ImageArtwork *image = dynamic_cast<Artworks::ImageArtwork*>(artwork);
    VERIFY(image != nullptr, "Cannot convert artwork to image");

    VERIFY(expectedKeywords == keywords, "Keywords are not the same!");
    VERIFY(image->getImageSize().width() == 4167, "Image width was read incorrectly");
    VERIFY(image->getImageSize().height() == 4167, "Image height was read incorrectly");

    return 0;
}
