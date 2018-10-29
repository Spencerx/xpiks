#ifndef ARTWORKSUPDATERMOCK_H
#define ARTWORKSUPDATERMOCK_H

#include <cstddef>
#include <memory>
#include <vector>

#include <QDebug>
#include <QVector>

#include "Artworks/artworkmetadata.h"
#include "Artworks/artworkssnapshot.h"
#include "Common/logging.h"
#include "Common/types.h"
#include "Services/iartworksupdater.h"

namespace Mocks {
    class ArtworksUpdaterMock: public Services::IArtworksUpdater {
        // IArtworksUpdater interface
    public:
        virtual void updateArtwork(std::shared_ptr<Artworks::ArtworkMetadata> const &artwork) override {
            LOG_DEBUG << "Updated artwork";
            m_UpdatedIDs.push_back(artwork->getItemID());
        }

        virtual void updateArtworkByID(Common::ID_t id, size_t, QVector<int> const &rolesToUpdate = QVector<int>()) override {
            LOG_DEBUG << "Updated by id";
            m_UpdatedIDs.push_back(id);
        }

        virtual void updateArtworks(Artworks::ArtworksSnapshot const &snapshot,
                                    Services::UpdateMode) override {
            LOG_DEBUG << "Updated snapshot";
            for (auto &item: snapshot) {
                m_UpdatedIDs.push_back(item->getItemID());
            }
        }

    public:
        bool isUpdated(Common::ID_t const &id) {
            for (auto &updatedID: m_UpdatedIDs) {
                if (updatedID == id) {
                    return true;
                }
            }
            return false;
        }

    private:
        std::vector<Common::ID_t> m_UpdatedIDs;
    };
}

#endif // ARTWORKSUPDATERMOCK_H
