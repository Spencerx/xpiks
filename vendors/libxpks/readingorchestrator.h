#ifndef READINGORCHESTRATOR_H
#define READINGORCHESTRATOR_H

namespace Artworks {
    class ArtworksSnapshot;
}

namespace MetadataIO {
    class MetadataReadingHub;
}

namespace Models {
    class SettingsModel;
}

namespace libxpks {
    namespace io {
        class ReadingOrchestrator
        {
        public:
            explicit ReadingOrchestrator(MetadataIO::MetadataReadingHub &readingHub,
                                         Models::SettingsModel &settingsModel);
            virtual ~ReadingOrchestrator();

        public:
            void startReading();

        private:
            void startReadingImages();
            void startReadingVideos();

        private:
            Artworks::ArtworksSnapshot const &m_ItemsToReadSnapshot;
            MetadataIO::MetadataReadingHub &m_ReadingHub;
            Models::SettingsModel &m_SettingsModel;
        };
    }
}

#endif // READINGORCHESTRATOR_H
