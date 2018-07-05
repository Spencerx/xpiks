#ifndef WRITINGORCHESTRATOR_H
#define WRITINGORCHESTRATOR_H

#include <QVector>
#include <QMutex>
#include <Artworks/artworkssnapshot.h>

namespace Models {
    class ArtworkMetadata;
}

namespace Helpers {
    class AsyncCoordinator;
}

namespace libxpks {
    namespace io {
        class WritingOrchestrator
        {
        public:
            explicit WritingOrchestrator(const Artworks::ArtworksSnapshot &artworksToWrite,
                                         Helpers::AsyncCoordinator *asyncCoordinator,
                                         Models::SettingsModel *settingsModel);
            virtual ~WritingOrchestrator();

        public:
            void startWriting(bool useBackups, bool useDirectExport=true);
            void startMetadataWiping(bool useBackups);

        private:
            void startWritingImages(Artworks::ArtworksSnapshot::Container &rawSnapshot, bool useBackups, bool useDirectExport);
            void startWritingVideos(Artworks::ArtworksSnapshot::Container &rawSnapshot, bool useBackups, bool useDirectExport);
            void startWipingImages(Artworks::ArtworksSnapshot::Container &rawSnapshot, bool useBackups);

        private:
            const Artworks::ArtworksSnapshot &m_ItemsToWriteSnapshot;
            Models::SettingsModel *m_SettingsModel;
            Helpers::AsyncCoordinator *m_AsyncCoordinator;
        };
    }
}

#endif // WRITINGORCHESTRATOR_H
