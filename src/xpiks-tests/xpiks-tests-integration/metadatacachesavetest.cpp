#include "metadatacachesavetest.h"
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
#include "../../xpiks-qt/MetadataIO/metadataioservice.h"
#include "../../xpiks-qt/MetadataIO/metadataioworker.h"
#include "../../xpiks-qt/MetadataIO/metadatacache.h"
#include "../../xpiks-qt/MetadataIO/cachedartwork.h"
#include "testshelpers.h"

QString MetadataCacheSaveTest::testName() {
    return QLatin1String("MetadataCacheSaveTest");
}

void MetadataCacheSaveTest::setup() {
    Models::SettingsModel *settingsModel = m_TestsApp.getSettingsModel();
    m_TestsApp.getSettingsModel().setAutoFindVectors(false);
}

int MetadataCacheSaveTest::doTest() {
    Models::ArtItemsModel *artItemsModel = m_TestsApp.getArtItemsModel();
    QList<QUrl> files;
    files << setupFilePathForTest("images-for-tests/pixmap/img_0007.jpg")
          << setupFilePathForTest("images-for-tests/pixmap/seagull-for-clear.jpg")
          << setupFilePathForTest("images-for-tests/pixmap/seagull.jpg")
          << setupFilePathForTest("images-for-tests/vector/026.jpg")
          << setupFilePathForTest("images-for-tests/vector/027.jpg")
          << setupFilePathForTest("images-for-tests/mixed/0267.jpg");

    MetadataIO::MetadataIOCoordinator *ioCoordinator = m_TestsApp.getMetadataIOCoordinator();
    SignalWaiter waiter;
    QObject::connect(ioCoordinator, SIGNAL(metadataReadingFinished()), &waiter, SIGNAL(finished()));

    MetadataIO::MetadataIOService *metadataIOService = m_TestsApp.getMetadataIOService();
    MetadataIO::MetadataIOWorker *worker = metadataIOService->getWorker();
    MetadataIO::MetadataCache &metadataCache = worker->getMetadataCache();

    VERIFY(metadataCache.retrieveRecordsCount() == 0, "Metadata cache is not empty on startup");

    int addedCount = artItemsModel->addLocalArtworks(files);
    VERIFY(addedCount == files.length(), "Failed to add file");
    ioCoordinator->continueReading(true);

    VERIFY(waiter.wait(20), "Timeout exceeded for reading metadata.");

    VERIFY(!ioCoordinator->getHasErrors(), "Errors in IO Coordinator while reading");

    const int desiredCount = files.count();

    sleepWaitUntil(5, [&metadataCache, &desiredCount]() {
        return metadataCache.retrieveRecordsCount() == desiredCount;
    });

    VERIFY(metadataCache.retrieveRecordsCount() == desiredCount, "Metadata cache was not filled in time");

    QVector<MetadataIO::CachedArtwork> cachedArtworks;
    metadataCache.dumpToArray(cachedArtworks);

    VERIFY(m_TestsApp.getArtworksCount() == cachedArtworks.count(), "Metadata cache size does not match");
    for (MetadataIO::CachedArtwork &ca: cachedArtworks) {
        Artworks::ArtworkMetadata *artwork = artItemsModel->findArtworkByFilepath(ca.m_Filepath);
        VERIFY(artwork != nullptr, "Metadata cache contains orphanned artworks");

        VERIFY(artwork->getTitle() == ca.m_Title, "Title does not match");
        VERIFY(artwork->getDescription() == ca.m_Description, "Description does not match");
        VERIFY(artwork->getKeywords() == ca.m_Keywords, "Keywords do not match");
    }

    return 0;
}
