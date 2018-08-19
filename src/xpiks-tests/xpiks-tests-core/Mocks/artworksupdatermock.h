#ifndef ARTWORKSUPDATERMOCK_H
#define ARTWORKSUPDATERMOCK_H

#include <Services/iartworksupdater.h>
#include <Common/logging.h>

namespace Mocks {
    class ArtworksUpdaterMock: public Services::IArtworksUpdater {
        // IArtworksUpdater interface
    public:
        virtual void updateArtwork(std::shared_ptr<Artworks::ArtworkMetadata> const &) override {
            LOG_DEBUG << "Updated artwork";
        }

        virtual void updateArtworkByID(Common::ID_t, size_t, QVector<int> const &rolesToUpdate = QVector<int>()) override {
            LOG_DEBUG << "Updated by id";
        }

        virtual void updateArtworks(Artworks::ArtworksSnapshot const &,
                                    Services::UpdateMode) override {
            LOG_DEBUG << "Updated snapshot";
        }
    };
}

#endif // ARTWORKSUPDATERMOCK_H
