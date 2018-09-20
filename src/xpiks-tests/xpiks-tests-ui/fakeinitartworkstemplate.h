#ifndef FAKEINITARTWORKSTEMPLATE_H
#define FAKEINITARTWORKSTEMPLATE_H

#include <Commands/Base/icommandtemplate.h>

namespace Artworks {
    class ArtworksSnapshot;
}

class FakeInitArtworksTemplate: public Commands::ICommandTemplate<Artworks::ArtworksSnapshot>
{
public:
    virtual void execute(Artworks::ArtworksSnapshot const &artworks) override;
};

#endif // FAKEINITARTWORKSTEMPLATE_H
