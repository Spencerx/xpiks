#include "zipartworkstest.h"
#include <QUrl>
#include <QFileInfo>
#include <QStringList>
#include "integrationtestbase.h"
#include "signalwaiter.h"
#include "../../xpiks-qt/Commands/commandmanager.h"
#include "../../xpiks-qt/Models/artitemsmodel.h"
#include "../../xpiks-qt/MetadataIO/metadataiocoordinator.h"
#include "../../xpiks-qt/Models/artworkmetadata.h"
#include "../../xpiks-qt/Models/settingsmodel.h"
#include "../../xpiks-qt/Models/filteredartitemsproxymodel.h"
#include "../../xpiks-qt/Models/ziparchiver.h"
#include "../../xpiks-qt/Helpers/filehelpers.h"
#include "../../xpiks-qt/Models/imageartwork.h"

QString ZipArtworksTest::testName() {
    return QLatin1String("ZipArtworksTest");
}

void ZipArtworksTest::setup() {
    Models::SettingsModel *settingsModel = m_TestsApp.getSettingsModel();
    m_TestsApp.getSettingsModel().setAutoFindVectors(true);
}

int ZipArtworksTest::doTest() {
    Models::ArtItemsModel *artItemsModel = m_TestsApp.getArtItemsModel();
    QList<QUrl> files;
    files << setupFilePathForTest("images-for-tests/vector/026.jpg");
    files << setupFilePathForTest("images-for-tests/vector/027.jpg");
    files << setupFilePathForTest("images-for-tests/pixmap/seagull.jpg");

    MetadataIO::MetadataIOCoordinator *ioCoordinator = m_TestsApp.getMetadataIOCoordinator();
    SignalWaiter waiter;
    QObject::connect(ioCoordinator, SIGNAL(metadataReadingFinished()), &waiter, SIGNAL(finished()));

    int addedCount = artItemsModel->addLocalArtworks(files);
    VERIFY(addedCount == files.length(), "Failed to add files");
    ioCoordinator->continueReading(true);

    VERIFY(waiter.wait(20), "Timeout exceeded for reading metadata.");

    VERIFY(!ioCoordinator->getHasErrors(), "Errors in IO Coordinator while reading");

    Models::FilteredArtItemsProxyModel *filteredModel = m_TestsApp.getFilteredArtItemsModel();
    filteredModel->selectFilteredArtworks();
    filteredModel->setSelectedForZipping();

    Models::ZipArchiver *zipArchiver = m_TestsApp.getZipArchiver();

    QObject::connect(zipArchiver, SIGNAL(finishedProcessing()), &waiter, SIGNAL(finished()));

    zipArchiver->archiveArtworks();

    VERIFY(waiter.wait(20), "Timeout while zipping artworks");

    VERIFY(zipArchiver->getItemsCount() == 2, "ZipArchiver didn't get all the files");
    VERIFY(!zipArchiver->getHasErrors(), "Errors while zipping");

    for (int i = 0; i < files.length(); ++i) {
        Artworks::ArtworkMetadata *artwork = m_TestsApp.getArtwork(i);
        Artworks::ImageArtwork *image = dynamic_cast<Artworks::ImageArtwork*>(artwork);
        Q_ASSERT(image != nullptr);

        if (image->hasVectorAttached()) {
            QString zipPath = Helpers::getArchivePath(artwork->getFilepath());
            VERIFY(QFileInfo(zipPath).exists(), "Zip file not found");
        }
    }

    return 0;
}


