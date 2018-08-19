#include "importlostmetadatatest.h"
#include <QUrl>
#include <QList>
#include <QDebug>
#include "integrationtestbase.h"
#include "signalwaiter.h"
#include "testshelpers.h"
#include "xpikstestsapp.h"
#include <MetadataIO/metadataioworker.h>
#include <MetadataIO/metadatacache.h>
#include <Artworks/imageartwork.h>

QString ImportLostMetadataTest::testName() {
    return QLatin1String("ImportLostMetadataTest");
}

void ImportLostMetadataTest::setup() {
    m_TestsApp.getSettingsModel().setAutoFindVectors(false);
}

int ImportLostMetadataTest::doTest() {
    QList<QUrl> files;
    files << setupFilePathForTest("images-for-tests/read-only/Nokota_Horses.jpg");

    VERIFY(m_TestsApp.addFilesForTest(files), "Failed to add files");

    auto artwork = m_TestsApp.getArtwork(0);
    const QString filepath = artwork->getFilepath();
    VERIFY(artwork->getKeywords().count() == 0, "Initial keywords should not be found");

    QStringList keywordsToCheck = QStringList() << "some" << "random" << "keywords";

    auto &filteredArtworksListModel = m_TestsApp.getFilteredArtworksModel();
    for (auto &keyword: keywordsToCheck) {
        filteredArtworksListModel.appendKeyword(0, keyword);
    }

    MetadataIO::MetadataIOService &metadataIOService = m_TestsApp.getMetadataIOService();
    MetadataIO::MetadataCache &cache = metadataIOService.getWorker()->getMetadataCache();

    // wait for artwork backup request and metadata cache timer
    sleepWaitUntil(10, [&]() {
        auto fakeArtwork = std::make_shared<Artworks::ArtworkMetadata>(filepath, 12345, 0);
        MetadataIO::CachedArtwork cachedArtwork;
        bool anythingAvailable = cache.read(fakeArtwork, cachedArtwork);
        return anythingAvailable && (cachedArtwork.m_Keywords == keywordsToCheck);
    });

    {
        auto tempFakeArtwork = std::make_shared<Artworks::ArtworkMetadata>(filepath, artwork->getItemID(), artwork->getDirectoryID());
        MetadataIO::CachedArtwork tempCachedArtwork;
        bool anythingAvailable = cache.read(tempFakeArtwork, tempCachedArtwork);
        VERIFY(anythingAvailable, "Artwork has not been saved to cache");
        VERIFY(tempCachedArtwork.m_Keywords == keywordsToCheck, "Saved keywords do not match real keywords");
    }

    // ----------- now that we know that cache is saved --------------

    qInfo() << "Now cache is saved after initial import";

    std::shared_ptr<Artworks::ArtworkMetadata> fakeArtworkToRead(
                new Artworks::ImageArtwork(filepath, artwork->getItemID(), artwork->getDirectoryID()),
                [](Artworks::ImageArtwork *artwork) {
        artwork->deleteLater();
    });

    Artworks::ArtworksSnapshot snapshotToRead;
    snapshotToRead.append(fakeArtworkToRead);

    auto &metadataIOCoordinator = m_TestsApp.getMetadataIOCoordinator();
    quint32 batchID = INVALID_BATCH_ID;
    SignalWaiter waiter;
    m_TestsApp.connectWaiterForImport(waiter);

    metadataIOCoordinator.readMetadataExifTool(snapshotToRead, batchID);
    metadataIOCoordinator.continueReading(false);

    VERIFY(waiter.wait(20), "Timeout exceeded for reading metadata second time.");
    VERIFY(m_TestsApp.checkImportSucceeded(), "Failed to read metadata");
    VERIFY(fakeArtworkToRead->areKeywordsEmpty(), "Keywords are not empty after exiftool read");

    qInfo() << "Secondary import has just finished";

    // wait for the metadata cache to receive new values
    // wait any pending writes to storage
    sleepWaitUntil(5, [&metadataIOService] {
        return !metadataIOService.isBusy();
    });
    metadataIOService.waitWorkerIdle();

    auto tempFakeArtwork = std::make_shared<Artworks::ArtworkMetadata>(filepath, artwork->getItemID(), artwork->getDirectoryID());
    MetadataIO::CachedArtwork tempCachedArtwork;
    bool anythingAvailable = cache.read(tempFakeArtwork, tempCachedArtwork);
    VERIFY(anythingAvailable, "Artwork has not been saved to cache");
    VERIFY(!tempCachedArtwork.m_Keywords.empty(), "Keywords are not empty after adding to library after import");

    return 0;
}
