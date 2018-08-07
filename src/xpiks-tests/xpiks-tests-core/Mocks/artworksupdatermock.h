#ifndef ARTWORKSUPDATERMOCK_H
#define ARTWORKSUPDATERMOCK_H

#include <Services/iartworksupdater.h>
#include <Common/logging.h>

namespace Mocks {
    class ArtworksUpdaterMock: public Services::IArtworksUpdater {
        // IArtworksUpdater interface
    public:
        virtual void updateArtworks(const Artworks::ArtworksSnapshot &, Services::UpdateMode) override {
            LOG_DEBUG << "Updated";
        }
    };
}

#endif // ARTWORKSUPDATERMOCK_H
