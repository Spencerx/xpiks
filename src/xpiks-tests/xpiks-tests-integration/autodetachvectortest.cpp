#include "autodetachvectortest.h"
#include <QUrl>
#include <QList>
#include <QFile>
#include <QDebug>
#include "integrationtestbase.h"
#include "signalwaiter.h"
#include "xpikstestsapp.h"

QString AutoDetachVectorTest::testName() {
    return QLatin1String("AutoDetachVectorTest");
}

void AutoDetachVectorTest::setup() {
    m_TestsApp.getSettingsModel().setAutoFindVectors(true);

    // copy file
    setupFilePathForTest("images-for-tests/items-to-remove/026.eps");
}

int AutoDetachVectorTest::doTest() {
    QList<QUrl> files;
    files << setupFilePathForTest("images-for-tests/items-to-remove/026.jpg");

    VERIFY(m_TestsApp.addFilesForTest(files), "Failed to add files");

    Artworks::ArtworkMetadata *metadata = m_TestsApp.getArtwork(0);
    Artworks::ImageArtwork *image = dynamic_cast<Artworks::ImageArtwork *>(metadata);
    VERIFY(image != NULL && image->hasVectorAttached(), "Vector is not attached!");

    const QString vectorPath = image->getAttachedVectorPath();
    QFile vectorFile(vectorPath);

    SignalWaiter unavailableVectorsWaiter;
    QObject::connect(&m_TestsApp.getArtworksListModel(), &Models::ArtworksListModel::unavailableVectorsFound,
                     &unavailableVectorsWaiter, &SignalWaiter::finished);

    VERIFY(vectorFile.exists(), "Vector can't be found on disk");
    VERIFY(vectorFile.remove(), "Failed to remove vector");
    qDebug() << "Removed vector file" << vectorPath;

    VERIFY(unavailableVectorsWaiter.wait(5), "Timeout exceeded for unavailable vectors");

    VERIFY(!image->hasVectorAttached(), "Vector should not be attached!");

    return 0;
}


