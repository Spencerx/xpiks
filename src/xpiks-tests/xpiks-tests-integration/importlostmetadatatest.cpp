#include "importlostmetadatatest.h"
#include <QUrl>
#include <QFileInfo>
#include <QStringList>
#include <QDebug>
#include "integrationtestbase.h"
#include "signalwaiter.h"
#include "../../xpiks-qt/Commands/commandmanager.h"
#include "../../xpiks-qt/Models/artitemsmodel.h"
#include "../../xpiks-qt/MetadataIO/metadataiocoordinator.h"
#include "../../xpiks-qt/Models/artworkmetadata.h"
#include "../../xpiks-qt/Models/settingsmodel.h"
#include "../../xpiks-qt/Models/imageartwork.h"
#include "../../xpiks-qt/MetadataIO/metadataioservice.h"
#include "../../xpiks-qt/MetadataIO/metadatacache.h"
#include "../../xpiks-qt/MetadataIO/metadataioworker.h"
#include "../../xpiks-qt/MetadataIO/metadataiocoordinator.h"
#include "../../xpiks-qt/Models/imageartwork.h"

QString ImportLostMetadataTest::testName() {
    return QLatin1String("ImportLostMetadataTest");
}

void ImportLostMetadataTest::setup() {
    Models::SettingsModel *settingsModel = m_CommandManager->getSettingsModel();
    settingsModel->setAutoFindVectors(false);
}

int ImportLostMetadataTest::doTest() {
    Models::ArtItemsModel *artItemsModel = m_CommandManager->getArtItemsModel();
    QList<QUrl> files;
    files << getFilePathForTest("images-for-tests/read-only/Nokota_Horses.jpg");

    MetadataIO::MetadataIOCoordinator *ioCoordinator = m_CommandManager->getMetadataIOCoordinator();
    SignalWaiter waiter;
    QObject::connect(ioCoordinator, SIGNAL(metadataReadingFinished()), &waiter, SIGNAL(finished()));

    int addedCount = artItemsModel->addLocalArtworks(files);
    VERIFY(addedCount == files.length(), "Failed to add file");
    ioCoordinator->continueReading(true);

    if (!waiter.wait(20)) {
        VERIFY(false, "Timeout exceeded for reading metadata.");
    }

    VERIFY(!ioCoordinator->getHasErrors(), "Errors in IO Coordinator while reading");

    Models::ArtworkMetadata *artwork = artItemsModel->getArtwork(0);
    const QString filepath = artwork->getFilepath();
    VERIFY(artwork->getKeywords().count() == 0, "Initial keywords should not be found");

    QStringList keywordsToCheck = QStringList() << "some" << "random" << "keyowrds";

    for (auto &keyword: keywordsToCheck) {
        artItemsModel->appendKeyword(0, keyword);
    }

    // wait for artwork to run justChanged() timer
    QThread::sleep(1);

    MetadataIO::MetadataIOService *metadataIOService = m_CommandManager->getMetadataIOService();

    // wait for metadata cache to flush
    QThread::sleep(3);

    MetadataIO::MetadataCache &cache = metadataIOService->getWorker()->getMetadataCache();

    sleepWaitUntil(4, [&]() {
        Models::ArtworkMetadata fakeArtwork(filepath, 12345, 0);
        MetadataIO::CachedArtwork cachedArtwork;
        bool anythingAvailable = cache.read(&fakeArtwork, cachedArtwork);
        fakeArtwork.release();
        return anythingAvailable && (cachedArtwork.m_Keywords == keywordsToCheck);
    });

    {
        Models::ArtworkMetadata tempFakeArtwork(filepath, artwork->getItemID(), artwork->getDirectoryID());
        MetadataIO::CachedArtwork tempCachedArtwork;
        bool anythingAvailable = cache.read(&tempFakeArtwork, tempCachedArtwork);
        tempFakeArtwork.release();
        VERIFY(anythingAvailable, "Artwork has not been saved to cache");
        VERIFY(tempCachedArtwork.m_Keywords == keywordsToCheck, "Saved keywords do not match real keywords");
    }

    // ----------- now that we know that cache is saved --------------

    qInfo() << "Now cache is saved after initial import";

    std::shared_ptr<Models::ArtworkMetadata> fakeArtworkToRead(
                new Models::ImageArtwork(filepath, artwork->getItemID(), artwork->getDirectoryID()),
                [](Models::ImageArtwork *artwork) {
        if (artwork->release()) {
            delete artwork;
        } else {
            // leak artwork to overcome assert for hold
        }
    });

    MetadataIO::ArtworksSnapshot snapshotToRead;
    snapshotToRead.append(fakeArtworkToRead.get());

    auto *metadataIOCoordinator = m_CommandManager->getMetadataIOCoordinator();

    quint32 batchID = INVALID_BATCH_ID;

    if (metadataIOCoordinator != nullptr) {
        metadataIOCoordinator->readMetadataExifTool(snapshotToRead, batchID);
    }

    ioCoordinator->continueReading(false);

    if (!waiter.wait(20)) {
        VERIFY(false, "Timeout exceeded for reading metadata second time.");
    }

    VERIFY(!ioCoordinator->getHasErrors(), "Errors in IO Coordinator while reading");

    VERIFY(fakeArtworkToRead->areKeywordsEmpty(), "Keywords are not empty after exiftool read");

    qInfo() << "Secondary import has just finished";

    // wait for the metadata cache to receive new values
    // wait any pending writes to storage
    sleepWaitUntil(5, [&metadataIOService] {
        return !metadataIOService->isBusy();
    });
    metadataIOService->waitWorkerIdle();

    Models::ArtworkMetadata tempFakeArtwork(filepath, artwork->getItemID(), artwork->getDirectoryID());
    MetadataIO::CachedArtwork tempCachedArtwork;
    bool anythingAvailable = cache.read(&tempFakeArtwork, tempCachedArtwork);
    tempFakeArtwork.release();
    VERIFY(anythingAvailable, "Artwork has not been saved to cache");
    VERIFY(!tempCachedArtwork.m_Keywords.empty(), "Keywords are not empty after adding to library after import");

    return 0;
}
