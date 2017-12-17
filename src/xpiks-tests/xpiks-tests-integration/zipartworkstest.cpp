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
    Models::SettingsModel *settingsModel = m_CommandManager->getSettingsModel();
    settingsModel->setAutoFindVectors(true);
}

int ZipArtworksTest::doTest() {
    Models::ArtItemsModel *artItemsModel = m_CommandManager->getArtItemsModel();
    QList<QUrl> files;
    files << getFilePathForTest("images-for-tests/vector/026.jpg");
    files << getFilePathForTest("images-for-tests/vector/027.jpg");
    files << getFilePathForTest("images-for-tests/pixmap/seagull.jpg");

    MetadataIO::MetadataIOCoordinator *ioCoordinator = m_CommandManager->getMetadataIOCoordinator();
    SignalWaiter waiter;
    QObject::connect(ioCoordinator, SIGNAL(metadataReadingFinished()), &waiter, SIGNAL(finished()));

    int addedCount = artItemsModel->addLocalArtworks(files);
    VERIFY(addedCount == files.length(), "Failed to add files");
    ioCoordinator->continueReading(true);

    VERIFY(waiter.wait(20), "Timeout exceeded for reading metadata.");

    VERIFY(!ioCoordinator->getHasErrors(), "Errors in IO Coordinator while reading");

    Models::FilteredArtItemsProxyModel *filteredModel = m_CommandManager->getFilteredArtItemsModel();
    filteredModel->selectFilteredArtworks();
    filteredModel->setSelectedForZipping();

    Models::ZipArchiver *zipArchiver = m_CommandManager->getZipArchiver();

    QObject::connect(zipArchiver, SIGNAL(finishedProcessing()), &waiter, SIGNAL(finished()));

    zipArchiver->archiveArtworks();

    VERIFY(waiter.wait(20), "Timeout while zipping artworks");

    VERIFY(zipArchiver->getItemsCount() == 2, "ZipArchiver didn't get all the files");
    VERIFY(!zipArchiver->getHasErrors(), "Errors while zipping");

    for (int i = 0; i < files.length(); ++i) {
        Models::ArtworkMetadata *artwork = artItemsModel->getArtwork(i);
        Models::ImageArtwork *image = dynamic_cast<Models::ImageArtwork*>(artwork);
        Q_ASSERT(image != nullptr);

        if (image->hasVectorAttached()) {
            QString zipPath = Helpers::getArchivePath(artwork->getFilepath());
            VERIFY(QFileInfo(zipPath).exists(), "Zip file not found");
        }
    }

    return 0;
}


