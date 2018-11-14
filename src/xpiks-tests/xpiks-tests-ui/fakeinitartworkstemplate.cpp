#include "fakeinitartworkstemplate.h"

#include <memory>

#include "Artworks/artworkmetadata.h"
#include "Artworks/artworkssnapshot.h"

void FakeInitArtworksTemplate::execute(const Artworks::ArtworksSnapshot &artworks) {
    for (auto &artwork: artworks) {
        artwork->initAsEmpty();
    }
}
