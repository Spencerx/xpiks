#include "fakeinitartworkstemplate.h"
#include <Artworks/artworkssnapshot.h>
#include "Artworks/artworkmetadata.h"

void FakeInitArtworksTemplate::execute(const Artworks::ArtworksSnapshot &artworks) {
    for (auto &artwork: artworks) {
        artwork->initAsEmpty();
    }
}
