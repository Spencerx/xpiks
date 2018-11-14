#include "metadatacachesavetest.h"

#include <cstddef>
#include <memory>

#include <QLatin1String>
#include <QList>
#include <QStringList>
#include <QUrl>
#include <QVector>

#include "Artworks/artworkmetadata.h"
#include "MetadataIO/cachedartwork.h"
#include "MetadataIO/metadataioservice.h"
#include "MetadataIO/metadataioworker.h"
#include "Models/Artworks/artworkslistmodel.h"
#include "Models/settingsmodel.h"

#include "testshelpers.h"  // IWYU pragma: keep
#include "xpikstestsapp.h"

namespace MetadataIO {
    class MetadataCache;
}

QString MetadataCacheSaveTest::testName() {
    return QLatin1String("MetadataCacheSaveTest");
}

void MetadataCacheSaveTest::setup() {
    m_TestsApp.getSettingsModel().setAutoFindVectors(false);
}

std::shared_ptr<Artworks::ArtworkMetadata> findArtworkByFilepath(Models::ArtworksListModel &artworksList, QString const &filepath) {
    const size_t size = artworksList.getArtworksSize();
    for (size_t i = 0; i < size; i++) {
        std::shared_ptr<Artworks::ArtworkMetadata> artwork;
        if (artworksList.tryGetArtwork(i, artwork)) {
            if (artwork->getFilepath() == filepath) {
                return artwork;
            }
        }
    }
    return std::shared_ptr<Artworks::ArtworkMetadata>();
}

int MetadataCacheSaveTest::doTest() {
    QList<QUrl> files;
    files << setupFilePathForTest("images-for-tests/pixmap/img_0007.jpg")
          << setupFilePathForTest("images-for-tests/pixmap/seagull-for-clear.jpg")
          << setupFilePathForTest("images-for-tests/pixmap/seagull.jpg")
          << setupFilePathForTest("images-for-tests/vector/026.jpg")
          << setupFilePathForTest("images-for-tests/vector/027.jpg")
          << setupFilePathForTest("images-for-tests/mixed/0267.jpg");

    MetadataIO::MetadataIOService &metadataIOService = m_TestsApp.getMetadataIOService();
    MetadataIO::MetadataIOWorker *worker = metadataIOService.getWorker();
    MetadataIO::MetadataCache &metadataCache = worker->getMetadataCache();
    VERIFY(metadataCache.retrieveRecordsCount() == 0, "Metadata cache is not empty on startup");

    VERIFY(m_TestsApp.addFilesForTest(files), "Failed to add files");

    const int desiredCount = files.count();

    sleepWaitUntil(5, [&metadataCache, &desiredCount]() {
        return metadataCache.retrieveRecordsCount() == desiredCount;
    });

    VERIFY(metadataCache.retrieveRecordsCount() == desiredCount, "Metadata cache was not filled in time");

    QVector<MetadataIO::CachedArtwork> cachedArtworks;
    metadataCache.dumpToArray(cachedArtworks);

    VERIFY(m_TestsApp.getArtworksCount() == cachedArtworks.count(), "Metadata cache size does not match");
    for (MetadataIO::CachedArtwork &ca: cachedArtworks) {
        auto artwork = findArtworkByFilepath(m_TestsApp.getArtworksListModel(), ca.m_Filepath);
        VERIFY(artwork != nullptr, "Metadata cache contains orphanned artworks");

        VERIFY(artwork->getTitle() == ca.m_Title, "Title does not match");
        VERIFY(artwork->getDescription() == ca.m_Description, "Description does not match");
        VERIFY(artwork->getKeywords() == ca.m_Keywords, "Keywords do not match");
    }

    return 0;
}
