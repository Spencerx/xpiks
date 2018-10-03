#ifndef CURRENTARTWORKSOURCEMOCK_H
#define CURRENTARTWORKSOURCEMOCK_H

#include <memory>
#include <Models/Editing/icurrenteditable.h>
#include <Models/Editing/currenteditableartwork.h>
#include <Commands/Base/icommandtemplate.h>
#include <Artworks/artworkmetadata.h>

namespace Mocks {

    class CurrentArtworkSourceMock: public Models::ICurrentEditableSource {
    public:
        using IArtworksCommandTemplate = Commands::ICommandTemplate<Artworks::ArtworksSnapshot>;

        CurrentArtworkSourceMock(std::shared_ptr<Artworks::ArtworkMetadata> const &artwork,
                                 const std::shared_ptr<IArtworksCommandTemplate> &updateTemplate=std::shared_ptr<IArtworksCommandTemplate>()):
            m_Artwork(artwork),
            m_UpdateTemplate(updateTemplate)
        { }

        // ICurrentEditableSource interface
    public:
        virtual std::shared_ptr<Models::ICurrentEditable> getCurrentEditable() const override {
            return std::make_shared<Models::CurrentEditableArtwork>(m_Artwork, m_UpdateTemplate);
        }

    private:
        std::shared_ptr<Artworks::ArtworkMetadata> m_Artwork;
        std::shared_ptr<IArtworksCommandTemplate> m_UpdateTemplate;
    };
}

#endif // CURRENTARTWORKSOURCEMOCK_H
